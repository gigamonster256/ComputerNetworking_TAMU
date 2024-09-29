#include "udp/server.hpp"

#include <iostream>

#include "tftp/packets.hpp"

using namespace udp;
using namespace tftp;

int main() {
  Server server;
  server.set_port(69)
      .set_max_clients(1)
      .add_handler(
          [](Client* client, const char* msg, size_t, client_data_ptr_t) {
            const Packet* packet = reinterpret_cast<const Packet*>(msg);
            std::cout << *packet << std::endl;
            // first message should be a RRQ
            if (packet->opcode != Opcode::RRQ) {
              return;
            }
            // try to open the file
            FILE* file = fopen(packet->payload.rq.filename(), "r");
            if (file == nullptr) {
              Packet error(ERROR(ErrorCode::FILE_NOT_FOUND));
              client->write(reinterpret_cast<void*>(&error), error.size());
              return;
            }
            // loop with no timeout
            block_num block = 1;
            while (true) {
              // read the file
              char data[TFTP_MAX_DATA_LEN];
              size_t length = fread(data, 1, TFTP_MAX_DATA_LEN, file);
              if (length == 0) {
                break;
              }
              // send the data
              Packet data_packet(DATA(block, data, length));
              client->write(reinterpret_cast<void*>(&data_packet),
                            data_packet.size());
              // wait for the ack
              char ack[TFTP_MAX_PACKET_LEN];
              size_t ack_length = client->read(ack, TFTP_MAX_PACKET_LEN);
              if (ack_length == 0) {
                break;
              }
              const Packet* ack_packet = reinterpret_cast<const Packet*>(ack);
              if (ack_packet->opcode != Opcode::ACK ||
                  ack_packet->payload.ack.get_block() != block) {
                break;
              }
              block++;
            }
            // close the file
            fclose(file);
          })
      .exec();
  return 0;
}