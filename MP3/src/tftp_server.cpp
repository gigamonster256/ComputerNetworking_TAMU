#include <sys/select.h>

#include <iostream>
#include <sstream>

#include "netascii.hpp"
#include "tftp/packets.hpp"
#include "udp/server.hpp"

using namespace udp;
using namespace tftp;

#define MAX_TIMEOUTS 5
#define TIMEOUT_SECONDS 10
#define MAX_CLIENTS 5

int main() {
  Server server;
  server.set_port(8080)
      .set_max_clients(MAX_CLIENTS)
      .add_handler([](Client* client, const char* msg, size_t,
                      client_data_ptr_t) {
        const Packet* packet = reinterpret_cast<const Packet*>(msg);
        switch (packet->opcode) {
          // Read request
          case Opcode::RRQ: {
            std::istream* file =
                new std::ifstream(packet->payload.rq.filename());
            if (!file->good()) {
              Packet error(ERROR(ErrorCode::FILE_NOT_FOUND));
              client->write(reinterpret_cast<void*>(&error), error.size());
              return;
            }
            // figure out the mode
            Mode rrq_mode = Mode::from_string(packet->payload.rq.mode());
            // convert to netascii if needed
            std::istream* old_file_ptr = nullptr;
            if (rrq_mode == Mode::Value::NETASCII) {
              old_file_ptr = file;
              file = new UNIXtoNetasciiStream(*file);
            }
            // loop over blocks
            block_num block = 1;
            int timeouts = 0;
            bool retry = false;
            size_t data_length = 0;
            char data[TFTP_MAX_DATA_LEN];
            bool max_data = false;
            while (timeouts < MAX_TIMEOUTS) {
              // read in file in chunks
              if (!retry) {
                file->read(data, TFTP_MAX_DATA_LEN);
                data_length = file->gcount();
                // finished reading the file and last block was less than max
                if (data_length == 0 && !max_data) {
                  break;
                }
                max_data = data_length == TFTP_MAX_DATA_LEN;
              }
              retry = false;

              // send the data
              Packet data_packet(DATA(block, data, data_length));
              client->write(
                  reinterpret_cast<void*>(&data_packet),
                  sizeof(Packet::opcode) + sizeof(block_num) + data_length);

            wait_for_ack:
              // timeout
              int client_fd = client->get_fd();
              fd_set read_fds;
              FD_ZERO(&read_fds);
              FD_SET(client_fd, &read_fds);
              struct timeval timeout;
              timeout.tv_sec = TIMEOUT_SECONDS;
              timeout.tv_usec = 0;
              int ret =
                  select(client_fd + 1, &read_fds, nullptr, nullptr, &timeout);
              if (ret == -1) {
                if (errno == EINTR) {
                  continue;
                }
                perror("RRQ select");
                break;
              }

              // didnt get an ack back
              if (ret == 0) {
                timeouts++;
                retry = true;
                continue;
              }

              timeouts = 0;

              char ack[TFTP_MAX_PACKET_LEN];
              client->read(ack, TFTP_MAX_PACKET_LEN);

              const Packet* ack_packet = reinterpret_cast<const Packet*>(ack);
              if (ack_packet->opcode != Opcode::ACK) {
                break;
              }
              // repeat ACK
              if (ack_packet->payload.ack.get_block() != block) {
                // do not retry (sorcerer's apprentice problem)
                goto wait_for_ack;
              }
              block++;
            }
            // clean up
            delete file;
            if (old_file_ptr != nullptr) {
              delete old_file_ptr;
            }
            return;
          }
          // client is sending us a file
          case Opcode::WRQ: {
            // check if file already exists
            std::ifstream check_file(packet->payload.rq.filename());
            if (check_file.is_open()) {
              Packet error(ERROR(ErrorCode::FILE_ALREADY_EXISTS));
              client->write(reinterpret_cast<void*>(&error), error.size());
              return;
            }
            check_file.close();

            // write data from client to string stream
            // read from buf to file
            Mode wrq_mode = Mode::from_string(packet->payload.rq.mode());
            std::stringstream ss;
            std::istream* buf = nullptr;
            if (wrq_mode == Mode::Value::NETASCII) {
              std::cerr << "NETASCII" << std::endl;
              buf = new NetasciitoUNIXStream(ss);
            } else {
              buf = &ss;
            }

            // open file in write mode
            std::ofstream file(packet->payload.rq.filename());

            Packet ack(ACK(0));
            client->write(reinterpret_cast<void*>(&ack), ack.size());

            block_num block = 1;
            int timeouts = 0;
            while (timeouts < MAX_TIMEOUTS) {
              // timeout
              int client_fd = client->get_fd();
              fd_set read_fds;
              FD_ZERO(&read_fds);
              FD_SET(client_fd, &read_fds);
              struct timeval timeout;
              timeout.tv_sec = TIMEOUT_SECONDS;
              timeout.tv_usec = 0;
              int ret =
                  select(client_fd + 1, &read_fds, nullptr, nullptr, &timeout);
              if (ret == -1) {
                if (errno == EINTR) {
                  continue;
                }
                perror("WRQ select");
                break;
              }
              // no data back from the client... resend the ack
              if (ret == 0) {
                timeouts++;
                Packet ack(ACK(block - 1));
                client->write(reinterpret_cast<void*>(&ack), ack.size());
                continue;
              }
              timeouts = 0;
              // read the data
              char data[TFTP_MAX_PACKET_LEN];
              size_t data_length = client->read(data, TFTP_MAX_PACKET_LEN);
              // check the data
              const Packet* data_packet = reinterpret_cast<const Packet*>(data);
              // not data
              if (data_packet->opcode != Opcode::DATA) {
                break;
              }
              // repeat data block (our ack was lost)
              if (data_packet->payload.data.get_block() != block) {
                Packet ack(ACK(block - 1));
                client->write(reinterpret_cast<void*>(&ack), ack.size());
                continue;
              }
              // write the data to ss
              ss.write(
                  data_packet->payload.data.get_data(),
                  data_length - sizeof(Packet::opcode) - sizeof(block_num));
              // send the ack
              Packet ack(ACK(block));
              client->write(reinterpret_cast<void*>(&ack), ack.size());

              // if the data is less than the max size, we are done
              if (data_length < TFTP_MAX_PACKET_LEN) {
                break;
              }

              // see how many characters are in the buffer
              size_t buf_size = ss.tellp();
              // write the buffer to the file
              buf->read(data, buf_size - 257);
              file.write(data, buf_size - 257);

              block++;
            }
            // write the remaining data to the file
            char data[TFTP_MAX_PACKET_LEN];
            buf->read(data, TFTP_MAX_PACKET_LEN);
            auto bytesRead = buf->gcount();
            file.write(data, bytesRead);

            if (wrq_mode == Mode::Value::NETASCII) {
              delete buf;
            }
            return;
          }
          default: {
            Packet error(ERROR(ErrorCode::ILLEGAL_OPERATION));
            client->write(reinterpret_cast<void*>(&error), error.size());
            return;
          }
        }
      })
      .exec();
  return 0;
}