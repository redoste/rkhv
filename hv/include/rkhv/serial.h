#ifndef COMMON_SERIAL_H
#define COMMON_SERIAL_H

#define COM_DATA_REG          0
#define COM_BAUD_LSB_REG      0
#define COM_INT_ENABLE_REG    1
#define COM_BAUD_MSB_REG      1
#define COM_INT_ID_REG        2
#define COM_FIFO_CONTROL_REG  2
#define COM_LINE_CONTROL_REG  3
#define COM_MODEM_CONTROL_REG 4
#define COM_LINE_STATUS_REG   5
#define COM_MODEM_STATUS_REG  6
#define COM_SCRATCH_REG       7
#define COM_PORTS_LEN         8

#define COM1                   0x3F8
#define COM1_DATA_REG          (COM1 + COM_DATA_REG)
#define COM1_BAUD_LSB_REG      (COM1 + COM_BAUD_LSB_REG)
#define COM1_INT_ENABLE_REG    (COM1 + COM_INT_ENABLE_REG)
#define COM1_BAUD_MSB_REG      (COM1 + COM_BAUD_MSB_REG)
#define COM1_INT_ID_REG        (COM1 + COM_INT_ID_REG)
#define COM1_FIFO_CONTROL_REG  (COM1 + COM_FIFO_CONTROL_REG)
#define COM1_LINE_CONTROL_REG  (COM1 + COM_LINE_CONTROL_REG)
#define COM1_MODEM_CONTROL_REG (COM1 + COM_MODEM_CONTROL_REG)
#define COM1_LINE_STATUS_REG   (COM1 + COM_LINE_STATUS_REG)
#define COM1_MODEM_STATUS_REG  (COM1 + COM_MODEM_STATUS_REG)
#define COM1_SCRATCH_REG       (COM1 + COM_SCRATCH_REG)

#define COM2                   0x2F8
#define COM2_DATA_REG          (COM2 + COM_DATA_REG)
#define COM2_BAUD_LSB_REG      (COM2 + COM_BAUD_LSB_REG)
#define COM2_INT_ENABLE_REG    (COM2 + COM_INT_ENABLE_REG)
#define COM2_BAUD_MSB_REG      (COM2 + COM_BAUD_MSB_REG)
#define COM2_INT_ID_REG        (COM2 + COM_INT_ID_REG)
#define COM2_FIFO_CONTROL_REG  (COM2 + COM_FIFO_CONTROL_REG)
#define COM2_LINE_CONTROL_REG  (COM2 + COM_LINE_CONTROL_REG)
#define COM2_MODEM_CONTROL_REG (COM2 + COM_MODEM_CONTROL_REG)
#define COM2_LINE_STATUS_REG   (COM2 + COM_LINE_STATUS_REG)
#define COM2_MODEM_STATUS_REG  (COM2 + COM_MODEM_STATUS_REG)
#define COM2_SCRATCH_REG       (COM2 + COM_SCRATCH_REG)

#define COM_BAUD_RATE_BASE 115200

#define COM_INT_ENABLE_DATA_AVAILABLE 0x01
#define COM_INT_ENABLE_TRANSMIT_EMPTY 0x02
#define COM_INT_ENABLE_BREAK_OR_ERROR 0x04
#define COM_INT_ENABLE_STATUS_CHANGE  0x08

#define COM_FIFO_CONTROL_ENABLE        0x01
#define COM_FIFO_CONTROL_CLEAR_RX_FIFO 0x02
#define COM_FIFO_CONTROL_CLEAR_TX_FIFO 0x04
#define COM_FIFO_CONTROL_DMA_MODE      0x08
#define COM_FIFO_CONTROL_TL_MASK       0xC0

#define COM_LINE_CONTROL_DLAB         0x80
#define COM_LINE_CONTROL_5BITS        0x00
#define COM_LINE_CONTROL_6BITS        0x01
#define COM_LINE_CONTROL_7BITS        0x02
#define COM_LINE_CONTROL_8BITS        0x03
#define COM_LINE_CONTROL_2_STOP_BITS  0x04
#define COM_LINE_CONTROL_PARITY_NONE  0x00
#define COM_LINE_CONTROL_PARITY_ODD   0x08
#define COM_LINE_CONTROL_PARITY_EVEN  0x18
#define COM_LINE_CONTROL_PARITY_MARK  0x28
#define COM_LINE_CONTROL_PARITY_SPACE 0x38

#define COM_MODEM_CONTROL_DTR_PIN 0x01
#define COM_MODEM_CONTROL_RTS_PIN 0x02
#define COM_MODEM_CONTROL_IRQ     0x08
#define COM_MODEM_CONTROL_LOOP    0x10

#define COM_LINE_STATUS_DATA_READY                         0x01
#define COM_LINE_STATUS_OVERRUN_ERROR                      0x02
#define COM_LINE_STATUS_PARITY_ERROR                       0x04
#define COM_LINE_STATUS_FRAMING_ERROR                      0x08
#define COM_LINE_STATUS_BREAK_INDICATOR                    0x10
#define COM_LINE_STATUS_TRANSMITTER_HOLDING_REGISTER_EMPTY 0x20
#define COM_LINE_STATUS_TRANSMITTER_EMPTY                  0x40
#define COM_LINE_STATUS_IMPENDING_ERROR                    0x80

#endif
