#include "udp/server.hpp"

#include <sys/select.h>

#include <iostream>

#include "tftp/packets.hpp"

using namespace udp;
using namespace tftp;

#define MAX_TIMEOUTS 5
#define TIMEOUT_SECONDS 10
#define MAX_CLIENTS 5

int main() {
  Server server;
  server.set_port(TFTP_PORT)
      .set_max_clients(MAX_CLIENTS)
      .add_handler([](Client* client, const char* msg, size_t,
                      client_data_ptr_t) {
        const Packet* packet = reinterpret_cast<const Packet*>(msg);
        switch (packet->opcode) {
          // Read request
          case Opcode::RRQ: {
            FILE* file = fopen(packet->payload.rq.filename(), "r");
            if (file == nullptr) {
              Packet error(ERROR(ErrorCode::FILE_NOT_FOUND));
              client->write(reinterpret_cast<void*>(&error), error.size());
              return;
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
                data_length = fread(data, 1, TFTP_MAX_DATA_LEN, file);
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
            fclose(file);
            return;
          }
          // client is sending us a file
          case Opcode::WRQ: {
            FILE* file = fopen(packet->payload.rq.filename(), "r");
            if (file != nullptr) {
              Packet error(ERROR(ErrorCode::FILE_ALREADY_EXISTS));
              client->write(reinterpret_cast<void*>(&error), error.size());
              return;
            }

            file = fopen(packet->payload.rq.filename(), "w");

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
              if (data_packet->payload.data.get_block() == block - 1) {
                Packet ack(ACK(block - 1));
                client->write(reinterpret_cast<void*>(&ack), ack.size());
                continue;
              }
              // wrong block number
              if (data_packet->payload.data.get_block() != block) {
                break;
              }
              // write the data
              fwrite(data_packet->payload.data.get_data(), 1,
                     data_length - sizeof(Packet::opcode) - sizeof(block_num),
                     file);
              // send the ack
              Packet ack(ACK(block));
              client->write(reinterpret_cast<void*>(&ack), ack.size());

              // if the data is less than the max size, we are done
              if (data_length < TFTP_MAX_PACKET_LEN) {
                break;
              }

              block++;
            }
            fclose(file);
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