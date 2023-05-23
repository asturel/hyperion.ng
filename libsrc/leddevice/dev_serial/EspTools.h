/* EspTools.h
*
*  MIT License
*
*  Copyright (c) 2023 awawa-dev
*
*  Project homesite: https://github.com/awawa-dev/HyperHDR
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.

*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
 */

#ifndef ESPTOOLS_H
#define ESPTOOLS_H

#include <utils/WaitTime.h>
#include <QSerialPortInfo>

class EspTools
{
	public:

	static void goingSleep(QSerialPort& _rs232Port)
	{
		uint8_t comBuffer[] = { 0x41, 0x77, 0x41, 0x2a, 0xa2, 0x35, 0x68, 0x79, 0x70, 0x65, 0x72, 0x68, 0x64, 0x72 };
		_rs232Port.write((char*)comBuffer, sizeof(comBuffer));
	}

	static void initializeEsp(QSerialPort& _rs232Port, QSerialPortInfo& serialPortInfo, Logger*& _log)
	{
		uint8_t comBuffer[] = { 0x41, 0x77, 0x41, 0x2a, 0xa2, 0x15, 0x68, 0x79, 0x70, 0x65, 0x72, 0x68, 0x64, 0x72 };

		if (serialPortInfo.productIdentifier() == 0xa && serialPortInfo.vendorIdentifier() == 0x2e8a)
		{
			Warning(_log, "Detected Rp2040 type board. HyperHDR skips the reset. State: %i, %i",
				_rs232Port.isDataTerminalReady(), _rs232Port.isRequestToSend());

			_rs232Port.write((char*)comBuffer, sizeof(comBuffer));

			_rs232Port.setDataTerminalReady(true);
			_rs232Port.setRequestToSend(true);
			_rs232Port.setRequestToSend(false);
		}
		else if (serialPortInfo.productIdentifier() == 0x80c2 && serialPortInfo.vendorIdentifier() == 0x303a)
		{
			Warning(_log, "Detected ESP32-S2 lolin mini type board. HyperHDR skips the reset. State: %i, %i",
				_rs232Port.isDataTerminalReady(), _rs232Port.isRequestToSend());

			_rs232Port.write((char*)comBuffer, sizeof(comBuffer));

			_rs232Port.setDataTerminalReady(true);
			_rs232Port.setRequestToSend(true);
			_rs232Port.setRequestToSend(false);
		}
		else
		{
			// reset to defaults
			_rs232Port.setDataTerminalReady(true);
			_rs232Port.setRequestToSend(false);
			wait(50);

			// reset device
			_rs232Port.setDataTerminalReady(false);
			_rs232Port.setRequestToSend(true);
			wait(150);

			// resume device
			_rs232Port.setRequestToSend(false);
			wait(100);
		}
	}
};

#endif
