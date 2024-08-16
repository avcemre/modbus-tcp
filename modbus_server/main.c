#include <modbus/modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define SERVER_ID 1
#define PORT 1502
#define NUM_REGISTERS 10

int main(void) {
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;
    int rc;
    int server_socket;

    // Modbus TCP context oluşturma
    ctx = modbus_new_tcp("192.168.30.202", PORT);
    if (ctx == NULL) {
        fprintf(stderr, "Unable to allocate libmodbus context\n");
        return -1;
    }

    // Modbus register ve bit mapping oluşturma
    mb_mapping = modbus_mapping_new(0, 0, NUM_REGISTERS, 0);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    for(int i = 1; i < NUM_REGISTERS; i++)
    {
        mb_mapping->tab_registers[0] = 99;
        mb_mapping->tab_registers[i] = i * 10;
    }

    // Server soketini başlatma
    server_socket = modbus_tcp_listen(ctx, 1);
    if (server_socket == -1) {
        fprintf(stderr, "Unable to listen on TCP port\n");
        modbus_mapping_free(mb_mapping);
        modbus_free(ctx);
        return -1;
    }

    // Client bağlantısını kabul etme ve işleme
    for (;;) {
        modbus_set_slave(ctx, SERVER_ID);
        int client_socket = modbus_tcp_accept(ctx, &server_socket);
        if (client_socket == -1) {
            fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
            continue;
        }

        // İstekleri işleme
        for (;;) {
            uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
            rc = modbus_receive(ctx, query);
            if (rc > 0) {
                modbus_reply(ctx, query, rc, mb_mapping);
            } else if (rc == -1) {
                // Bağlantı kaybedildi, döngüden çık ve yeni bir client kabul et
                break;
            }
        }
        close(client_socket);
    }

    // Kaynakları serbest bırakma
    close(server_socket);
    modbus_mapping_free(mb_mapping);
    modbus_free(ctx);

    return 0;
}

