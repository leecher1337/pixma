What is this?
-------------
Some little tools to unpack firmware of Canon PIXMA series for analysis.

Motivation
----------
Some Pixma printer is unable to reset the Waste Ink counter and refuses to print now,
even though waste ink container has been swapped out.
So I was curious about the inner workings of the firmware to find a clue on how to
reset the counter.
But I soon gave up on it. Nevertheless, it has been interesting and maybe this is
useful for other researchers? Basically, I just implemented what has been described at
https://www.contextis.com/en/blog/hacking-canon-pixma-printers-doomed-encryption

How to use?
-----------
1) Download firmware
Check the USB device ID of the Pixma printer, i.e. 04a9:1769
Then downlod the firmware by placing the device ID instead of 1769 in the following command:

  wget `wget -O - -o /dev/null http://gdlp01.c-wss.com/rmds/ij/ijd/ijdupdate/1769.xml | sed -n 's:.*<url>\(.*\)</url>.*:\1:p'`

2) Now decrypt firmware, and decode SREC file to binary
   You need srec_cat from srecord tools for this, i.e. srecord-1.64:  http://srecord.sourceforge.net/

  ./pixma_decrypt 1769V1100AN.bin decoded.asc
  grep -v -e '^SF' decoded.asc | srec_cat -o decoded.bin -binary

3) Now you can try to unpack the payload:
  ./pixma_unpack decoded.bin firmware.bin


Decrypt sdata.bin
-----------------
If you want to decrypt certificate data (for whatever reason you want to do this...
It's just a certificate list for trusted CAs, so no idea why this even has been "crypted"):

  wget http://dtv-p.c-ij.com/sdata/struct01/sdata.bin
  ./dec_sdata sdata.bin sdata.dec

