#include <CircularBuffer.h>
#include <USBComposite.h>

#define DEBUG false

USBMassStorage MassStorage;

#define PRODUCT_ID 0x29

// store incoming characters in a virtual buffer
CircularBuffer<uint8_t, 1024> buffer;

#include "image.h"

bool write(const uint8_t *writebuff, uint32_t memoryOffset, uint16_t transferLength) 
{
  if (DEBUG)
  {
    Serial1.print("Write: @ ");
    Serial1.print(memoryOffset);
    Serial1.print(" x ");
    Serial1.print(transferLength);
    Serial1.println();
  }

  for (int tx = 0; tx < transferLength; tx++)
  {
    uint32_t tx_offset = tx * SCSI_BLOCK_SIZE;
    uint32_t lba = memoryOffset + tx;

    uint32_t to = tx_offset;
    uint32_t i = 0;

    // if we are writing to block 4, interpret the data in the stream
    // we use a simple format here:
    //  + first character is the command
    //  + next 4 characters are the size (in readable ascii) of the data section -- 4 digits to be consistent with the read, but maxing out at 128
    //  + data to transmit follows
    
    switch (lba)
    {
      case 4:
        if (writebuff[tx_offset] == 't')
        {
          // transfer command
          
          if (DEBUG) Serial1.println("Transferring!");

          // interpret the size from the next 4 characters
          uint32_t tx_size = 
              (writebuff[tx_offset+1] - '0') * 1000
            + (writebuff[tx_offset+2] - '0') * 100
            + (writebuff[tx_offset+3] - '0') * 10
            + (writebuff[tx_offset+4] - '0') * 1;
          // cap the transfer at a max of 128 bytes
          if (tx_size > 128) tx_size = 128;

          if (DEBUG)
          {
            Serial1.print(" + ");
            Serial1.print(tx_size);
            Serial1.print(" bytes");
            Serial1.println();
  
            Serial1.print(" + [");
          }
          for (i = 0; i < tx_size; i++)
          {
            // print a maximum of 128 bytes to the serial port
            char to_print = writebuff[tx_offset+5+i];
            Serial1.print(to_print);
          }
          if (DEBUG)
          {
            Serial1.print("]");
            Serial1.println();
          }
        }
        break;
      default:
        break;
    }
  }
  
  return true;
}

bool read(uint8_t *readbuff, uint32_t memoryOffset, uint16_t transferLength) 
{
  if (DEBUG)
  {
    Serial1.print("Read: @ ");
    Serial1.print(memoryOffset);
    Serial1.print(" x ");
    Serial1.print(transferLength);
    Serial1.println();
  }
  
  memset(readbuff, 0, SCSI_BLOCK_SIZE*transferLength);

  for (int tx = 0; tx < transferLength; tx++)
  {
    uint32_t tx_offset = tx * SCSI_BLOCK_SIZE;
    uint32_t lba = memoryOffset + tx;

    uint32_t to = tx_offset;
    uint32_t i = 0;

    // we store the disk image in image.h. a very simple FAT12 image, with one 512 byte file.

    String data = "";
    char data_array[SCSI_BLOCK_SIZE];
    
    switch (lba)
    {
      case 0:
        memcpy(readbuff + tx_offset, disk_block0, ROMBLOCKDEVICE_BLOCK0_SIZE);
        break;
      case 1:
        memcpy(readbuff + tx_offset, disk_block1, ROMBLOCKDEVICE_BLOCK1_SIZE);
        break;
      case 3:
        memcpy(readbuff + tx_offset, disk_block3, ROMBLOCKDEVICE_BLOCK3_SIZE);
        break;
      case 4:
        // block 4 is our readme.htm file.

        // populate it dynamically with serial data if any is available.
        // wrap in html
        data += "<html><head/><body>";
        data += "<div id=\"serial.available\">";
        data += buffer.size();
        data += "</div>";
        data += "<div id=\"serial.data\">";

        i = 0;
        while ((i < 128) && (buffer.isEmpty() == false))
        {
          data += String(buffer.shift(), HEX);
          i++;
        }

        data += "</div>";
        data += "</body></html>\r\n";

        data.toCharArray(data_array, data.length() + 1);

        memcpy(readbuff + tx_offset, data_array, data.length() + 1);     
        break;
      default:
        break;
    }
  }
  
  return true;
}

void printInfo() {
  Serial1.println("Stats:");

  // virtual disk stats
  Serial1.print(" + ");
  Serial1.print(ROMBLOCKDEVICE_TOTAL_BLOCKS);
  Serial1.println(" blocks");

  // virtual disk block size
  Serial1.print(" + ");
  Serial1.print(SCSI_BLOCK_SIZE);
  Serial1.println(" bytes/transfer");

  // serial buffer stats
  Serial1.print(" + ");
  Serial1.print(buffer.size());
  Serial1.print(" / ");
  Serial1.print(buffer.capacity);
  Serial1.println(" bytes in buffer");
}

void setup() {
  USBComposite.setVendorId(0x0483);
  USBComposite.setProductId(0xA28A);

  MassStorage.setDriveData(0, ROMBLOCKDEVICE_TOTAL_BLOCKS, read, write);
  MassStorage.begin();

  Serial1.begin(115200);
  
  delay(2000);
}

void loop() {
  // service the mass storage device
  MassStorage.loop();

  // if any characters are available in the serial port buffer, transfer them to our buffer
  if (Serial1.available()) {
    uint8_t command = Serial1.read();
    buffer.push(command);
    switch (command)
    {
      // in debug mode, print some stats when we receive the 'i' chatacter
      case 'i': if (DEBUG) printInfo(); break;
      default: break;
    }
  }
}
