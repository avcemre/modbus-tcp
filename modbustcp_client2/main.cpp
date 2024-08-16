#include <stdio.h>
#include <modbus/modbus.h>
#include <errno.h>
#include <unistd.h>
#include <QProcess>

void change_IPAddress()
{
    QProcess process;
    //QString ipAddress = "%1.%2.%3.%4";
    QString ipAddress = "192.168.0.70";
    QString netMask = QString("255.255.255.0");
    QString command = QString("ifconfig eth1 %1").arg(ipAddress);
    process.start(command);
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError();
}

int main(void)
{
    change_IPAddress();
    int ret;
    modbus_t *ctx;
    int rc = 0;
    int i;
    uint16_t tab_reg[64];

    ctx = modbus_new_tcp("192.168.0.136", 502);

    ret = modbus_set_slave(ctx, 1);
    if(ret < 0){
        perror("modbus_set_slave error\n");
        return -1;
    }


    while(modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    }

    while(1)
    {
        //rc = modbus_read_input_registers(ctx, 0, 10, tab_reg);
        rc = modbus_read_registers(ctx, 0, 10, tab_reg);
        if(rc == -1)
        {
            fprintf(stderr, "%s\n", modbus_strerror(errno));
            break;
        }

        for(i=0; i < rc; i++)
        {
            printf("reg[%d]=%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
        }

        printf("-----------------------------------------------\n");
        sleep(1);
    }

    printf("freeing up resources\n");
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}
