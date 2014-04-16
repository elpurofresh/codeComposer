# invoke SourceDir generated makefile for msp430g2xx3_usci_i2c_slave_rx.p430
msp430g2xx3_usci_i2c_slave_rx.p430: .libraries,msp430g2xx3_usci_i2c_slave_rx.p430
.libraries,msp430g2xx3_usci_i2c_slave_rx.p430: package/cfg/msp430g2xx3_usci_i2c_slave_rx_p430.xdl
	$(MAKE) -f C:\andres\Code\codeComposer\msp430g2xx3_usci_i2c_slave_rx/src/makefile.libs

clean::
	$(MAKE) -f C:\andres\Code\codeComposer\msp430g2xx3_usci_i2c_slave_rx/src/makefile.libs clean

