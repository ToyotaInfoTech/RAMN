.. _ctf_writeups:

CTF Write-ups
=============

This page contains write-ups of past CTFs featuring RAMN. 
You can download past CTF firmware and challenge prompts from the `misc folder <https://github.com/ToyotaInfoTech/RAMN/tree/main/misc/past_CTFs>`_ of the github repository.
Feel free to contact us to add or remove your own write-ups.

DEFCON Embedded Systems Village 2024
------------------------------------

Participant Write-ups:

-  https://justinapplegate.me/2024/esvctf-playagame/

DEFCON Car Hacking Village 2024
-------------------------------

Flag format is flag{xxxx}. Ten RAMN boards were made available on a table at the CHV.
It was specified that each ECU had a different diagnostics interface (either USB, UDS, KWP2000, or XCP).
It was reminded that the Flash address range is 0x08000000-0x08040000 and that the RAM address range is 0x20000000-0x20040000.

This page contains simple write-ups. A jupyter notebook with very detailed solutions is available `in the misc folder <https://github.com/ToyotaInfoTech/RAMN/tree/main/misc/jupyter_notebooks>`_. 

[A] Secret Menu
^^^^^^^^^^^^^^^

.. code-block:: text

	Challenge Description: Look away while I input my password!
	Attachment: ECUA_REDACTED.elf

It can be immediately identified that ECU A is the one with the USB diagnostics interface because it is the only one with USB.
By browsing screens, we can identify that there is a debug screen waiting for a secret code.

.. image:: img/writeups/chv_ecua_1.jpg
   :align: center


We can load the provided firmware file in Ghidra. Because it is a .elf file with debug symbols, it is easy to reverse engineer.

Using Window > Defined Strings and looking for the "Awaiting secret code" string, then following references, we can identify that the debug mode is unlocked when the DEBUG_MODE_UNLOCKED variable is set to 1.

Following WRITE references, we can identify that this variable is set to 1 if the verify_secret_input function returns 1.

.. image:: img/writeups/chv_ecua_2.png
   :align: center
   
By analyzing the verify_secret_input function, we can identify which RAMN inputs will unlock the debug mode.

.. image:: img/writeups/chv_ecua_3.png
   :align: center
   
   
With help from cansniffer (or RAMN's CAN RX MONITOR screen), we can identify which CAN ID corresponds to each control, by physically moving controls and observing which CAN ID has a payload that changes accordingly.
We can then set RAMN's inputs as specified by the verify_secret_input.

.. image:: img/writeups/chv_ecua_4.jpg
   :align: center

The debug screen now shows that a new USB command interface can be accessed by typing "#", and that the current username has an incorrect CRC.

.. image:: img/writeups/chv_ecua_5.jpg
   :align: center
   
By typing "#" then "help" in the USB interface, we find that there is a "username" command that accepts one argument.  

By entering "default_user" into https://crccalc.com/ and finding the algorithm that yields C862ED4F, we identify that the CRC algorithm used is CRC-32/ISO-HDLC.
The flag can be retrieved by inputting a username with a CRC of 0xDA5D344D, which can be computed using tools such as `CRC RevEng <https://reveng.sourceforge.io/>`_ or `crchack <https://github.com/resilar/crchack>`_.

.. image:: img/writeups/chv_ecua_6.png
   :align: center
   

.. image:: img/writeups/chv_ecua_7.jpg
   :align: center



[B] Sit Next To Me
^^^^^^^^^^^^^^^^^^

.. code-block:: text

	Challenge Description: You wouldn't download a byte. 
	(Note: Flag will be transmitted once on ID 0x777 when challenge is solved.)
	
It can be identified that ECU B has an active XCP interface (at CAN IDs 0x552/0x553) by using scanning tools such as caringcaribou:

.. code-block:: bash

	caringcaribou xcp discovery -autoblacklist 10
	
.. image:: img/writeups/chv_ecub_1.png
   :align: center

Further probing with caringcaribou reveals that CAL/PAG resource is available but requires authentication.

.. code-block:: bash

	caringcaribou xcp info 0x552 0x553

.. image:: img/writeups/chv_ecub_2.png
   :align: center	
	
The command ``caringcaribou xcp commands 0x552 0x553`` can be used to identify that available commands are GET_STATUS, SYNCH, GET_SEED, UNLOCK, SET_MTA, UPLOAD, and DOWNLOAD.

It can be inferred from the prompt and from the information gathered so far that the flag will be transmitted with CAN ID 0x777 if the DOWNLOAD command (0xF0) is used - but this command requires authentication (using GET_SEED and UNLOCK).
	
We can use SET_MTA and UPLOAD commands to dump memory from the ECU. It can be observed with ``caringcaribou xcp dump 0x552 0x553 0x08000000 256`` that the flash range is not accessible, but that the RAM range is accessible (e.g., with ``caringcaribou xcp dump 0x552 0x553 0x20000000 256``). Note that some versions of caringcaribou might have bugs that will prevent this command from succeeding.

Caringcaribou does not support XCP authentication, so next steps should be done by directly sending CAN messages, following XCP specifications. A seed can be requested with ``cansend can0 552#f80001``.

.. image:: img/writeups/chv_ecub_3.png
   :align: center

This reveals that the seed is 6 bytes.
We can use the dump command to dump the full RAM address range (0x20000000-0x20040000) and search for the seed in it.
**Caringcaribou shouldn't be used (without modification) because it resets the XCP connection with each command, which resets the seed. The seed may be found at several locations (e.g., in the TRNG buffer), so all locations should be checked.**

To dump the RAM with XCP, we can use SET_MTA (0xF6) and UPLOAD (0xF5). For example, to dump 6 bytes from 0x20000000, we use:

.. code-block:: bash

	cansend can0 552#f6000020000000
	cansend can0 552#f506

Each successive call to UPLOAD will dump the next addresses (e.g., address 0x20000006 for the example above).
By dumping the whole RAM for different seeds, we can identify that the seed is consistently located near 0x20033f50.
It can also be observed that there is another 6-byte variable that changes just next to it. This can be identified as the potential expected answer to the seed. There is no authentication attempt limit, so we are free to try different permutations.

.. image:: img/writeups/chv_ecub_5.png
   :align: center

Flag can be read by requesting a seed, dumping the RAM to read the expected answer, unlocking the ECU with that answer, and using the DOWNLOAD command to ask the ECU to transmit the flag.

.. image:: img/writeups/chv_ecub_6.png
   :align: center

	
[C] Come again?
^^^^^^^^^^^^^^^

.. code-block:: text

	Challenge Description: The 2000s called; they want their ECU back.

The challenge prompt suggests that ECU C uses KWP2000. We can use caringcaribou to find ECUs using UDS and KWP2000:

.. code-block:: bash

	caringcaribou uds discovery --autoblacklist 10
	
.. image:: img/writeups/chv_ecuc_1.png
   :align: center
	
From RAMN's documentation, we can identify that 7e3 corresponds to ECU D's UDS interface and that 7e6 corresponds to ECU C's KWP2000 interface (and this can be confirmed by reading info with ReadDataByIdentifier).
For this challenge, we focus on 7e6/7ee (ECU C's KWP2000 interface).

Caringcaribou's service discovery reveals that many services are available:

.. code-block:: bash

	caringcaribou uds services 0x7e6 0x7ee

.. image:: img/writeups/chv_ecuc_2.png
   :align: center

The presence of service 0x1a indicates that this interface is KWP2000 and not UDS. Service 0x29 does not correspond to AUTHENTICATION, because this is not a UDS interface (and we technically shouldn't be using caringcaribou for it).

Trying to read all DIDs with ReadDataByIdentifier reveals that DID 0x0000 returns a "Security Access Denied". It can be inferred that the goal of the challenge is to bypass that security access.

.. image:: img/writeups/chv_ecuc_3.png
   :align: center
   
Trying to request a seed with the default session will return error 0x80, which for KWP2000 means that the service is not supported in current session.
We can bruteforce all sessions and observe that session 0x92 (KWP2000 extended session) is available.
We may however get other errors when requesting Security Access seeds: either "Time delay not required" (meaning we have to wait for the bruteforce protection timer to expire) or "Subfunction not supported".

By bruteforcing all security levels, we observe that security level 0x05 exists and returns a 16-bit seed:

.. image:: img/writeups/chv_ecuc_4.png
   :align: center

We can request as many seeds as we like, and they appear to be random.
Because the seed is only 16-bit long, bruteforcing appears to be the easiest approach.
However, the ECU will limit the number of attempts:

.. image:: img/writeups/chv_ecuc_5.png
   :align: center

Fortunately, it can be observed by poking around that the ECU resets the number of attempts whenever the "Diagnostic Session Control" service is called to request a new session, allowing us to try as many attempts as we want without having to reset the ECU.

We can therefore use the following script, which repetitively asks for a new seed and tries the answer "1234" (and should be stopped once it eventually gets lucky and unlocks the ECU).

.. code-block:: bash

	timeout 1000s bash -c 'while [ $SECONDS -lt 1000 ]; \
	do \
	echo "10 92" | isotpsend can0 -s 7e6 -d 7ee; \
	echo "27 05" | isotpsend can0 -s 7e6 -d 7ee; \
	echo "27 06 12 34" | isotpsend can0 -s 7e6 -d 7ee; \
	sleep 0.001
	done' 
	
(Note that this script is very slow but functional; it was expected from participants to write a more efficient script.)
After a few minutes, the ECU should be unlocked and the flag can be read with ReadDataByIdentifier with DID 0x0000:

.. code-block:: bash

	echo "22 00 00" | isotpsend can0 -s 7e6 -d 7ee

.. image:: img/writeups/chv_ecuc_6.png
   :align: center

[D] Light the way
^^^^^^^^^^^^^^^^^

.. code-block:: text

	Challenge Description: These LEDs were made for lighting.
	(Hint: dumpable firmware size is 0x0c548 bytes, don't spend your time looking for more.)
	
From the previous challenges, we know that ECU D has a UDS interface at 7e3/7eb.
We can use caringcaribou to scan available services:
	
.. code-block:: bash

	caringcaribou uds services 0x7e3 0x7eb

.. image:: img/writeups/chv_ecud_1.png
   :align: center

We can use the dump_dids module to read all DIDs:

.. code-block:: bash

	caringcaribou uds dump_dids 0x7e3 0x7eb
	
.. image:: img/writeups/chv_ecud_2.png
   :align: center
   
We can observe that WriteDataByIdentifier is active, and that the only DID that can be written to is DID 0x0207, with what appears to be an address in RAM.
We can try slightly modifying that value, and we observe that the LEDs on RAMN change as a result.
Because DID 0x206 says "LED CONTROL POINTER", and because the prompt and title suggest that the LEDs are involved, we can understand that this DID is used to specify the address in memory that is displayed on the LEDs.
We can also observe that it is possible to make that value point to flash addresses.

.. code-block:: bash

	echo "2E 02 07 20 02 00 00" | isotpsend can0 -s 7e3 -d 7eb 
	echo "2E 02 07 08 00 00 04" | isotpsend can0 -s 7e3 -d 7eb
	
We can therefore expect to be able to display the value of the flag, byte by byte, on the RAMN LEDs.
However, we still do not know the address of the flag.

We can observe that the REQUEST_UPLOAD and TRANSFER_DATA are active, which allow us to dump the firmware (see :ref:`request_upload`). The size of the firmware is specified in the challenge prompt: 0x0c548 bytes.

After dumping the firmware, we can open it in Ghidra (using the same settings as the one provided for ECUA_REDACTED.elf). Searching for "flag", we can find the string ``Loaded FLAG from private flash at address %p``, where %p is replaced by "0x0803e000".
We can therefore conclude that the flag is at 0x0803e000, and all we need to do is dump it byte by byte using the Write Data By Identifier service (The documentation can immediately identify which LED represents which bit, see :ref:`body_expansion`).

.. code-block:: bash

	# Point LEDs to first byte of flag
	echo "2E 02 07 08 03 e0 00" | isotpsend can0 -s 7e3 -d 7eb
	# Read byte by looking at the 8 LEDs on RAMN
	
	# Point LEDs to second byte
	echo "2E 02 07 08 03 e0 01" | isotpsend can0 -s 7e3 -d 7eb
	# Read next byte
	# etc...
	

Flag: flag{BEST_LIGHT_SHOW_IN_VEGAS}.

Automotive CTF Japan 2024
-------------------------

Participant Write-ups:

- https://laysakura.github.io/2024/09/14/automotive-ctf-2024-japan-final/ (JP)
- https://qiita.com/kusano_k/items/140d08521b9667cd6ab9 (JP)
- https://blog.hamayanhamayan.com/entry/2024/09/14/112907 (JP)
- https://emeth.jp/diary/2024/09/automotive-ctf-japan-writeup/ (JP)


Block Harbor VicOne Automotive CTF 2024
---------------------------------------

Flag format is bh{xxxx}. Each of the six teams was provided with two RAMN sets with CTF firmware, and there was an available reference RAMN with standard firmware shared between participants.
The letter in brackets in the challenge title indicates in which ECU the flag is located.

[FILE] SWD 1 (6 solves)
^^^^^^^^^^^^^^^^^^^^^^^

Intended difficulty: Easy.
Tags: Forensics.

.. code-block:: text

	The attached file is a logic analyzer capture of a RAMN ECU reprogramming session using an ST-LINK V2… 
	Can you figure out the plaintext (not obfuscated) flag embedded in firmware?
	
	
The provided file is a logic analyzer capture (from a Scanaquad SQ200).

.. code-block:: text

	Time;CH 1 SWCLK;CH 2 SWDIO
	0.000000000;1;1
	0.594872000;0;1
	0.594878000;1;1
	...

It can be loaded in tools such as PulseView to be decoded (the correct CSV format should be entered in the import options).
Once loaded, the signals can be decoded with the SWD protocol analyzer (as hinted by the title).

.. image:: img/writeups/SWD1.png
   :align: center

The decoded data can be exported in a text file.
Simply searching for the string "bh{" (62 68 7B in hex) in little-endian (7B6862) reveals the flag in plaintext.

.. image:: img/writeups/SWD1_2.png
   :align: center
   
Flag: bh{an4lyst_s3ssION_Ro4d}.
	
[FILE] SWD 2 (3 solves)
^^^^^^^^^^^^^^^^^^^^^^^

Intended difficulty: Very Difficult. 
Tags: Forensics, Reverse.	

.. code-block:: text

	The firmware of challenge “SWD 1” broadcasts every second two flags in plaintext over CAN, using the same function. 
	CAN ID 0x12345678 is used to broadcast the (non-obfuscated) flag of "SWD 1".
	The flag of this challenge is the one transmitted with ID 0x7777.
	(Note: Flash starts at 0x08000000, RAM at 0x20000000. Reset_Handler() is at 0x08001570).
	

	
This challenge follows SWD 1.
The first step is to extract the full firmware from the logic analyzer capture. This can be done by looking for “W AP4” commands (which indicate the address), and for “W APc” commands (which indicate the data to write at that address). 
Players must write a script to reconstruct a binary file of the firmware (Code FLASH starts at 0x08000000; data is written in 32-bit little-endian chunks).

.. image:: img/writeups/SWD2_1.png
   :align: center

After the firmware file is reconstructed, it can be loaded in Ghidra. Memory map must be set according to prompt for easier analysis.
	
.. image:: img/writeups/SWD2_2.png
   :align: center
   
Searching for 0x12345678 reveals the function that sends the flags. This can be used to understand that the second flag (when not obfuscated) is located at 0x20030020.	

.. image:: img/writeups/SWD2_3.png
   :align: center

There is only one other reference to 0x20030020 - it can be inferred that this is the function that loads the flag in it.

.. image:: img/writeups/SWD2_4.png
   :align: center
   
This reveals the function that deobfuscates the flag. 

.. image:: img/writeups/SWD2_5.png
   :align: center
   
Which core part was originally in C:

.. code-block:: C

	uint8_t SECRET_KEY[] = "dw\x01ss1";
	flag_addr[i] = (obfuscated_flag[i] ^ SECRET_KEY[i % key_length]) -  SECRET_KEY[i % key_length];
   
The only remaining step is to identify where the obfuscated flag is.
It is known from previous steps that 0x20037750 is where the obfuscated flag is in RAM.
The challenge prompt provides the location of the Reset_Handler() function:   
   
.. image:: img/writeups/SWD2_7.png
   :align: center

It can be deduced that the RAM default values are loaded from FLASH 0x0800ab94 to RAM 0x20037750 (the obfuscated flag coincidentally happened to be at the first address of the .data section, just before the flag of SWD1):

.. image:: img/writeups/SWD2_8.png
   :align: center

The steps above can be used to deobfuscate the flag as bh{pr0duct_AMB1tion}.

[D] Follow Me (6 solves)
^^^^^^^^^^^^^^^^^^^^^^^^

Intended difficulty: Easy. 
Tags: Hardware.

.. code-block:: text

	ECU D's LEDs are flickering when the engine key is on the “IGN” (rightmost) position. 
	We hope that you brought a logic analyzer to debug that…
	

LEDs are controlled by ECU D's SPI interface, and the SPI signals have clearly marked probes on the Body PCB.
Observing the SPI signal with a logic analyzer shows that ECU D normally updates the status of LEDs every 10 ms.	
	
.. image:: img/writeups/SPI.png
   :align: center
   
When the engine key is on the IGN position, it can be seen that there is a burst of data before the transmission of LED status - which is the flag in plaintext ASCII.

.. image:: img/writeups/SPI_2.png
   :align: center

Flag: bh{TREE_FORMS_WIND}.

   
[D] I2C (4 solves)
^^^^^^^^^^^^^^^^^^^

Intended difficulty: Medium. 
Tags: Hardware.

.. code-block:: text

	This flag will be transmitted every second on CAN with ID 0x778
	if you can send any byte to ECU D on its I2C interface (port I2C2, address 0x63).
	Note: I2C pins have internal pull-up resistors. 



This challenge requires the player to read the `STM32L5x2 datasheet <https://www.st.com/resource/en/datasheet/stm32l552cc.pdf>`_ and identify possible pins for the I2C2 port.
	
SDA could be at PF0, PB11, or PB14; SCL could be at PF1, PB10, or PB13.
PB13 is already used by the SPI interface of the "Follow Me" challenge, and PF0/PF1 are not available on the 48-pin package used by RAMN.
This only leaves SDA:PB11/SCL:PB10 and SDA:PB14/SCL:PB10 as possible configurations to try on the board (with the former being the correct one).
This can be attempted with any I2C tool, such as an FT2232H board configured in I2C mode.
	
.. code-block:: python

	from pyftdi.i2c import I2cController

	i2c = I2cController()
	i2c.configure('ftdi://ftdi:2232h/1',frequency=10000)
	slave = i2c.get_port(0x63)
	print(slave.exchange([0xFF], 1))

Which triggers the transmission of the flag.

.. image:: img/writeups/i2c.png
   :align: center	
   
Flag: bh{INFAMOUS_REMAKE}.
	
[D] Forgotten Field (4 solves)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Intended difficulty: Medium/Difficult. 
Tags: CAN, Hardware.

.. code-block:: text

	Many tools consider a CAN frame consists of arbitration, control, and data fields. 
	ID 0x607 thinks they should check some more.

The challenge prompt suggests that there is a "forgotten field" not displayed by most CAN tools such as candump.
A quick look at the CAN page on `Wikipedia <https://en.wikipedia.org/wiki/CAN_bus#/media/File:CAN-bus-frame-with-stuff-bit-and-correct-CRC.png>`_ shows that this is likely a reference to the CRC field (further hinted by the "check some" in the prompt).

The solution is therefore to look at the CRC field of CAN frames with ID 0x607.
The easiest way to do this is by looking at the CAN frames with a logic analyzer (it is easier if you turn off other ECUs, and look at the TX pin of ECU D directly instead of CANH/CANL).
An alternate way to solve this is to reconstruct CAN frames based on the data from candump (note that you must reproduce bit-stuffing before computing the CRC15 of the CAN protocol).

.. image:: img/writeups/CRC.png
   :align: center	
   
The flag is simply the CRC of the CAN frames (one byte per frame): bh{LAGGING_BEHIND}.  

.. image:: img/writeups/CRC2.png
   :align: center	
 
	
[C] CVE-2017-14937 (4 solves)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Intended difficulty: Easy. 
Tags: CAN, Hardware.

.. code-block:: text

	Go write something at 0x1111 and read something at 0x0000.
	
As the title implies, this challenge is a simple reproduction of CVE-2017-14937. CVE-2017-14937 details how the ECU's Security Access service can be used to unlock the ECU.
Once the ECU is unlocked, the player only needs to use the WriteDataByIdentifier service to write any data at DID 0x1111, which will allow them to read the flag using the ReadDataByIdentifier service with DID 0x0000.

Detailed participant write up available `here <https://laysakura.github.io/2024/10/24/automotive-ctf-2024-world-final/#c-cve-2017-14937>`_.

Flag: bh{SUP3RS0NIc}.
	
[B] Rush Hour (3 solves)
^^^^^^^^^^^^^^^^^^^^^^^^

Intended difficulty: Medium. 
Tags: CAN, Hardware.

.. code-block:: text

	We added a UDS disable feature to ECU B to prevent you from reading the flag. 
	Note: Flag is redacted in attached firmware - you must extract the flag from an actual RAMN board.
	
This challenge provides ECU B's firmware (with a redacted flag) as an attachment. The firmware is in .elf format, with debug symbols, making it easy to reverse engineer.
(Another reason to provide the firmware in .elf format was to make it easier to know correct Ghidra settings for challenges where only a .hex file was provided).

Searching for the flag reveals that it can be read using the ReadDataByIdentifier UDS service with DID 0x0001 (appearing as 0x100 in Ghidra because of ARM32's endianness).
	
.. image:: img/writeups/Rushhour_0.png
   :align: center	
	
However, as specified in the challenge prompt, there is a global variable UDS_ENABLE that prevents the player from using UDS when it is set to 0.
	
.. image:: img/writeups/Rushhour_1.png
   :align: center	
	
The player should notice that the default value of this variable is 1 (meaning UDS is available) but that it is set to 0 during boot.	
	
.. image:: img/writeups/Rushhour_2.png
   :align: center	
		
Because it is set to 0 AFTER the CAN peripheral has been activated, there is a 10 ms window during which UDS can be used.
The solution is therefore to spam the request while the ECU is booting.

.. image:: img/writeups/Rushhour_3.png
   :align: center	
		
Flag: bh{Sl0W_Down_Every0ne}.
	
[A] slcan’t (2 solves)
^^^^^^^^^^^^^^^^^^^^^^

Intended difficulty: Medium. 
Tags: CAN, USB.

.. code-block:: text

	Why does ramn_utils.c need such a large ascii_hashmap? 
	We could use all those unused bytes to store a flag instead…

The table ascii_hashmap in ramn_utils.c (which code is available on github) is used to convert ASCII hexadecimal strings to bytes. 
Because hexadecimal characters only consist of "0 to 9", "A to F", and "a to f", the table is mostly filled with 0x00.

.. code-block:: C

	static const uint8_t ascii_hashmap[] =
	{
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  !"#$%&'
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ()*+,-./
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 01234567
			0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 89:;<=>?
			0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // @ABCDEFG
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HIJKLMNO
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // PQRSTUVW
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // XYZ[\]^_
			0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // `abcdefg
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // hijklmno
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // pqrstuvw
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // xyz{|}~.
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // ........
	};
	
	inline uint8_t ASCIItoUint8(const uint8_t* src)
	{
		return (ascii_hashmap[src[0]] << 4) + (ascii_hashmap[src[1]]);
	}
	


The challenge prompt suggests that the flag is located in that table.
Reading the source code shows that ASCIItoUint8 is used by the 't' command of the `slcan protocol <https://www.lawicel.com/can232/can232_v1.pdf>`_ when requesting the transmission of a CAN message using the slcan interface.

.. code-block:: C

	CANTxData[i++] = ASCIItoUint8(&USBRxBuffer[offset]);

The format of a transmit slcan command is t<id><dlc><data>.
A simple method to dump one byte from the table is to execute the slcan command t00210<index> to force ECU A to transmit the byte at <index> with CAN ID 0x002.
By repeating this command and observing the CAN bus at the same time (from an external CAN adapter), we can dump the entirety of the table - in which the flag is located.	
	

.. code-block:: python

	for i in range(0x100):
		command = bytes('t00210'.encode()) + (bytes([i])) + bytes('\r'.encode())
		ser.write(command)
		
.. code-block:: bash

	python -m can.logger -i pcan -c PCAN_USBBUS1 --filter 0x002:0x7FF

.. image:: img/writeups/slcant.png
   :align: center	
	
Flag: bh{B4RK_B0RK_bOrK}.

[C] DID not done (2 solves)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Intended difficulty: Medium/Difficult. 
Tags: UDS.

.. code-block:: text

	Flag is a 26-byte string at 0x0803e000 but Read Memory By Address won't let me read it :(

The challenge prompt gives the address and size of the flag.
Scanning the UDS services of ECU C reveals that the service DynamicallyDefineDataIdentifier is active.
This service can therefore be used to define a dynamic DID (which should be in the 0xF300-0xF3FF range according to UDS standard) at 0x0803e000 (with size 26).
The flag can then be obtained by reading that DID using ReadDataByIdentifier.

.. code-block:: bash   
   
   echo "2c 02 F3 00 14 08 03 e0 00 1A" | isotpsend can0 -s 7e2 -d 7ea 	
   echo "22 F3 00" | isotpsend can0 -s 7e2 -d 7ea 

Flag: bh{TAKE_THE_LONG_WAY_HOME}.
	
Participant `write-up available here <https://laysakura.github.io/2024/10/24/automotive-ctf-2024-world-final/#c-did-not-done>`_.


[A/C] Ramen Clicker (1 solve)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Intended difficulty: Medium/Difficulty. 
Tags: CAN, Hardware.

.. code-block:: text

	My high score is 0x9000.

The screen displays a "Ramen clicker" game, which counts up each time the SHIFT joystick is center-pressed.
The prompt suggests that the flag is displayed if the counter goes over 0x9000.

.. image:: img/writeups/ramen_clicker.jpg
   :align: center
   
By observing the CAN bus, it can be observed that there is no authentication between ECU A and ECU C, therefore it is trivial to spoof the state of the joystick.
Using cansniffer, we can observe that 045#0106 corresponds to "joystick pressed" and 045#0101 corresponds to "joystick released" (first byte corresponds to gear status and can be ignored).

Trying to spoof these messages will however trigger the following screen:

.. image:: img/writeups/ramen_monke.jpg
   :align: center	
  
The anti-cheat system is not punishing, and the game can be restarted without a power reset.
Because the firmware isn't provided, it is not clear what triggers the anti-cheat system.
However, because there is no proper authentication, we know that it should be possible to impersonate ECU C, as long as the impersonation is not obvious.

The first step is to exclude ECU C (normally transmitting the joystick messages) from the CAN bus, which can be done for example by:

- Using the expansion header of ECU C to short the reset pin (21) to the ground pin (4) (jumping wires were provided for that purpose).
- Using the USB serial interface to shut down ECU's C power supply.

After that, the player can just send 045#0106 and 045#0101 to increment clicks.

Note: The anti-cheat system is triggered when ECU A does not receive a message with CAN ID 045 for more than 500 ms, or when ECU A receives a message with ID 001 (ECU C sends a message with ID 001 to warn ECU A when it itself receives a message with ID 045 and therefore knows that someone is cheating). There is no message frequency checking.
It is not necessary to know these exact conditions, the player only needs to attempt a relatively clean transition between normal traffic and compromised traffic.

.. code-block:: bash

	#Turn off ECU C or hold it in reset mode, then immediately execute
	timeout 1000s bash -c 'while [ $SECONDS -lt 1000 ]; \
	do \
	cansend can0 045#0106; \
	cansend can0 045#0101; \
	done'

An alternative solution is to physically press the button 0x9000 times.

Flag: bh{N1NN1KUM4SHIMA5HI}.
	
	
[D] Security Access 1 (1 solve)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Intended difficulty: Very Difficult. 
Tags: Reverse, UDS, Hardware.

.. code-block:: text

	The attached file corresponds to the firmware of ECU D, with all its flags redacted.
	Try to extract the real value of bh{XXXXXXXXXXXXXXXXXXX} from your RAMN hardware.

The attached file is a .hex file, with no debug symbols and therefore a bit difficult to reverse engineer.
The .hex file is common for challenges "Security Access 1" and "Security Access 2".
A superficial analysis with Ghidra (loaded in ARM v8 LE mode) reveals that, as the title suggests, flags can be read with ReadDataByIdentifier (DID 0x0001 and DID 0x0002) after Security Access is performed (for levels 0x01 and 0x03, respectively).

Following references leads us to the Security Access algorithm.

.. image:: img/writeups/security_access_1_1.png
   :align: center	

Security Access check for "Security Access 1" is performed by FUN_0900be24, which can be identified as "memcmp" by AI tools such as ChatGPT.
We can conclude that 08002310h is where the address of the expected 16-byte (static) password is.

.. image:: img/writeups/security_access_1_2.png
   :align: center	
   
08002310h contains 0BF974C0h, but that address cannot be found in the firmware file.

Based on the address map of the `reference manual <https://www.st.com/resource/en/reference_manual/dm00346336-stm32l552xx-and-stm32l562xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf>`_, we can observe that 0BF974C0h is in the System memory bootloader area (in ROM). 
The trick is to identify that this value is in system memory and is therefore common to all STM32L552 microcontrollers (at least from the same batch), so it can be read from another ECU. Therefore, the password could be read:

- By using the ReadMemoryByAddress UDS Service of ECU C (which is not disabled, contrary to ECU D).
- Or, by using one of the many memory dump methods (UDS, FDCAN Bootloader, JTAG, etc.) of an unlocked RAMN with standard firmware (which was made available to participants).

With ECU C UDS:

.. code-block:: bash   
   
	echo "23 14 0B F9 74 C0 10" | isotpsend can0 -s 7e2 -d 7ea


.. image:: img/writeups/security_access_1_5.png
   :align: center	

With JTAG on ECU D of a reference RAMN:

.. image:: img/writeups/security_access_1_3.png
   :align: center	
   

Flag can be obtained by sending that password and reading DID 0x0001.   
   
.. code-block:: bash   
   
   echo "27 01" | isotpsend can0 -s 7e3 -d 7eb
   echo "27 02 40 49 6E 74 65 72 6E 61 6C 20 46 6C 61 73 68 20"  | isotpsend can0 -s 7e3 -d 7eb
   echo "22 00 01"  | isotpsend can0 -s 7e3 -d 7eb
   
.. image:: img/writeups/security_access_1_4.png
   :align: center	
   
Flag: bh{We_hAve_HSM_4t_Home}.

[D] Security Access 2 (1 solve)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Intended difficulty: Very Difficult. 
Tags: Reverse, UDS.

.. code-block:: text

	Same as “Security Access 1”, but you are looking for bh{YYYYYYYYYYYYYYYYYYY}.

Following the same steps as "Security Access 1", we can identify the function that checks the password.

.. image:: img/writeups/security_access_2_1.png
   :align: center	

This function compares the provided password to four 32-bit values, based on a function that uses as a parameter the strings "HAPPY HAPPY HAPPY HAPPY", "HAPPY HAPPY HAPPY", "HAPPY HAPPY", and "HAPPY" (with their respective size).

Following references, and with help from ChatGPT, we can identify that:

- FUN_080013a8 function reads from 080013b8h (which value is 20032A14h).
- 20032A14h is initialized by FUN_0800139c with parameter 20032820h.
- 20032820h is loaded from 08000c64h, which value is 40023000h.

Therefore, the password is read in 32-bit chunks from 40023000h. Reading the `reference manual <https://www.st.com/resource/en/reference_manual/dm00346336-stm32l552xx-and-stm32l562xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf>`_ again, we can identify that this address corresponds to a Special Function Register of the CRC engine peripheral.
Although we could follow references to reverse engineer the parameters of the engine (initialized at FUN_08003580), there is no attempt limits, therefore we can also simply try all common CRC32 algorithms (with different endianness).

We can use https://crccalc.com/ with the default STM32 CRC engine algorithm (CRC-32/MPEG-2), which gives us 0x14b311c9, 0x6442CA33, 0xC25DE077, and 0x6DA5F0C1, and corresponds to the correct password.

.. code-block:: bash   
   
    echo "27 03" | isotpsend can0 -s 7e3 -d 7eb
    echo "27 04 C9 11 B3 14 33 CA 42 64 77 E0 5D C2 C1 F0 A5 6D" | isotpsend can0 -s 7e3 -d 7eb
    echo "22 00 02"  | isotpsend can0 -s 7e3 -d 7eb


.. image:: img/writeups/security_access_2_2.png
   :align: center	

Flag: bh{Thanks_P3riPH3Rals!}.

 