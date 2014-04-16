## THIS IS A GENERATED FILE -- DO NOT EDIT
.configuro: .libraries,430 linker.cmd \
  package/cfg/msp430g2xx3_usci_i2c_slave_rx_p430.o430 \

linker.cmd: package/cfg/msp430g2xx3_usci_i2c_slave_rx_p430.xdl
	$(SED) 's"^\"\(package/cfg/msp430g2xx3_usci_i2c_slave_rx_p430cfg.cmd\)\"$""\"C:/andres/Code/codeComposer/msp430g2xx3_usci_i2c_slave_rx/Debug/configPkg/\1\""' package/cfg/msp430g2xx3_usci_i2c_slave_rx_p430.xdl > $@
