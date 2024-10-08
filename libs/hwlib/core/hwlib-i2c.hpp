// ==========================================================================
//
// File      : hwlib-i2c.hpp
// Part of   : C++ hwlib library for close-to-the-hardware OO programming
// Copyright : wouter@voti.nl 2017
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// ==========================================================================

// this file contains Doxygen lines
/// @file

#ifndef HWLIB_I2C_H
#define HWLIB_I2C_H

// #include <stdint.h>

// #include "hwlib-pin.hpp"
// #include "hwlib-wait.hpp"
#include<CleanRTOS.h>

namespace crt
{
	extern ILogger& logger;
}

namespace hwlib
{

   /// \brief
   /// i2c bus master interface
   /// \details
   /// This class abstracts the interface of a master to an I2C bus.
   ///
   /// In its simplest form, an I2C bus has one master and a number of slaves
   /// that are connected by two wires: SCL (clock) and SDA (data).
   /// Both lines are pulled up by pull-up resistor, and can (only)
   /// be pulled down by a connected chip (open-collector).
   ///
   /// \image html i2c-bus.png
   ///
   /// An I2C transaction is either a read transaction or a transaction.
   /// In both cases, the transaction starts with the master transmitting a
   /// control byte, which contains the address of the slave chip, and one bit
   /// that indicates whether it is a read or a write transaction.
   /// The bits of a byte are transferred MSB (most significant bit) first.
   ///
   /// \image html i2c-command-1.png
   ///
   /// \image html i2c-command-2.png
   ///
   /// Next the slave chip receives (write transaction) or transmits
   /// (read transaction) as many bytes as the master asks for.
   ///
   /// \image html i2c-write-read.png
   ///
   /// At the bit level, master generates clock pulses by pulling the
   /// SCL line low. While the SCL is low, the master or slave can put a bit
   /// on the SDA line by pulling it down (for a 0) or letting it float
   /// (for a 1).

   /// The SCL line is always driven by the master (unless the slave
   /// uses clock-stretching),
   /// the SDA line is driven by the device on the bus that sends the bit.
   ///
   /// \image html i2c-bit-level.png
   ///
   /// Two special conditions are used.
   /// To signal the start (S) of a transaction, the sda is pulled low while
   /// the clk is high. The reverse is used to indicate a stop
   /// (P, the end of a transaction):
   /// the dta is released (goes high) while the clock is high.
   ///
   /// \image html i2c-signaling.png
   ///
   /// Most slave chips that have only one data byte
   /// that can be read or written use
   /// a single-byte read or write transmission to read or write that data byte.
   /// Slave chips that have more than one address that can be written often
   /// use a write transaction where the first byte(s) written
   /// determine the address (within the slave chip),
   /// and the subsequent byte(s) are written to that address (and to
   /// the next addresses).
   /// An I2C read transaction addresses the slave chip, but has no provision
   /// to specify an address within the slave chip.
   /// A common trick is that a read addresses the last address specified by
   /// a (previous) write transaction.
   /// Hence to read from address X first a write is done to address X, but
   /// the transaaction stops after the X, hence nothing is written,
   /// but this sets the address pointer inside the slave chip.
   /// Now a read transaction reads from this address.
   ///
   /// As always, consult the datasheet of the chip for the details.
   ///
   /// The I2C bus was invented by Philips, who had a patent on it.
   /// Hence other manufacturers that implemented the I2C bus on their
   /// chips had either to pay royalties to Philips, or tried to avoid this
   /// by implementing a protocol that was compatible with I2C,
   /// without mentioning I2C. The I2C patent has expired,
   /// but you can still find many chips that are described as
   /// 'implementing a two-wire protocol' or something similar.
   /// In most cases this means that the chip implements I2C.
   ///
   /// references:
   /// - <a href="http://www.nxp.com/documents/user_manual/UM10204.pdf">
   ///    I2C bus specification and user manual</a> (pdf)
   /// - <a href="http://i2c.info/i2c-bus-specification">
   ///    I2C Bus Specification</A> (info page)
   /// - <a href="https://en.wikipedia.org/wiki/I2C">
   ///    I2C Bus</A> (wikipedia)
   ///

   class i2c_bus
   {
   public:
      /// \brief
      /// i2c write transaction
      /// \details
      /// This function write n bytes from data[] to the slave at address a.
      ///
      /// Note that n is a byte, hence the maximum number of bytes is 255.
      virtual void write(
          uint_fast8_t a,
          const uint8_t data[],
          size_t n) = 0;

      /// \brief
      /// i2c read transaction
      /// \details
      /// This function reads n bytes from the slave at address a to data[].
      ///
      /// Note that n is a byte, hence the maximum number of bytes is 255.
      virtual void read(
          uint_fast8_t a,
          uint8_t data[],
          size_t n) = 0;

      // Instead of write, one can opt for the more flexible (but error-prone) manual series:
      // write_begin, write_block (xn), write_end
      virtual void write_begin(
          uint_fast8_t a) = 0;

      virtual void write_block(
          const uint8_t data[],
          size_t n) = 0;

      virtual void write_end() = 0;

   }; // class i2c_bus

   /// \brief
   /// bit-banged i2c bus implementation
   /// \details
   /// This class implements a bit-banged master interface to an I2C bus.
   /// Limitations:
   ///    - only the 7-bit address format is supported
   ///    - clock stretching by the slave is not supporte
   ///    - only a single master is supported
   ///    - the speed is fixed at ~ 100 kHz or somewhat lower
   class i2c_bus_bit_banged_scl_sda : public i2c_bus
   {
   private:
      pin_oc &scl, &sda;

      void wait_half_period()
      {
         wait_us(1);
      }

      void write_bit(bool x)
      {
         scl.write(0);
         wait_half_period();
         sda.write(x);
         scl.write(1);
         wait_half_period();
      }

      bool read_bit()
      {
         scl.write(0);
         sda.write(1);
         wait_half_period();
         scl.write(1);
         wait_half_period();
         bool result = sda.read();
         wait_half_period();
         return result;
      }

      void write_start()
      {
         sda.write(0);
         wait_half_period();
         scl.write(0);
         wait_half_period();
      }

      void write_stop()
      {
         scl.write(0);
         wait_half_period();
         sda.write(0);
         wait_half_period();
         scl.write(1);
         wait_half_period();
         sda.write(1);
         wait_half_period();
      }

      bool read_ack()
      {
         bool ack = !read_bit();
         return ack;
      }

      void write_ack()
      {
         write_bit(0);
      }

      void write_nack()
      {
         write_bit(1);
      }

      void write_byte(uint_fast8_t x)
      {
         for (uint_fast8_t i = 0; i < 8; i++)
         {
            write_bit((x & 0x80) != 0);
            x = x << 1;
         }
      }

      uint_fast8_t read_byte()
      {
         uint_fast8_t result = 0;
         for (uint_fast8_t i = 0; i < 8; i++)
         {
            result = result << 1;
            if (read_bit())
            {
               result |= 0x01;
            }
         }
         return result;
      }

   public:
      /// \brief
      /// construct a bit-banged I2C bus from the scl and sda pins
      /// \details
      /// This constructor creates a bit-banged I2C bus master
      /// from the scl and sda pins.
      i2c_bus_bit_banged_scl_sda(pin_oc &scl, pin_oc &sda) : scl(scl), sda(sda)
      {
         scl.write(1);
         sda.write(1);
      }

      // Instead of write, one can opt for the more flexible (but error-prone) manual series:
      // write_begin, write_block (xn), write_end
      void write_begin(
          uint_fast8_t a) override
      {
         write_start();
         write_byte(a << 1);

        crt::logger.logText("begin write");
        crt::logger.logUint32(a);
      }

      void write_block(
          const uint8_t data[],
          size_t n) override
      {
         for (size_t i = 0; i < n; i++)
         {
            read_ack();
            write_byte(data[i]);

           crt::logger.logUint32(data[i]);
         }
      }

      void write_end() override
      {
         read_ack();
         write_stop();
        crt::logger.logText("end write");
         //(void)hwlib::now_us();
      }

      /// \brief
      /// write to a connect I2C slave device
      /// \details
      /// This function writes n bytes of data to the device with address a
      /// that is connected to the I2C bus.
      void write(
          uint_fast8_t a,
          const uint8_t data[],
          size_t n) override
      {
         write_begin(a);
         write_block(data, n);
         write_end();
         //(void)hwlib::now_us();
      }

      /// \brief
      /// read from a connected I2C slave device
      /// \details
      /// This function reads n bytes of data from the device with address a
      /// that is connected to the I2C bus.
      void read(
          uint_fast8_t a,
          uint8_t data[],
          size_t n) override
      {
        crt::logger.logText("begin read");
         write_start();
         write_byte((a << 1) | 0x01);
         read_ack();
         for (uint_fast8_t i = 0; i < n; i++)
         {
            if (i > 0)
            {
               write_ack();
            }
            data[i] = read_byte();
           crt::logger.logUint32(data[i]);
         }
         // Conclusie: na het laatste byte wordt geen write_ack() teruggestuurd naar de slave.
         // Is dat nooit nodig?
         write_stop();
        crt::logger.logText("end read");
      }

   }; // class i2c_bus_bit_banged_scl_sda

}; // namespace hwlib

#endif // HWLIB_I2C_H