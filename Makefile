

include mk/vars.mk



all:
	$(MAKE) -C $(SRC_DIR)

clean:
	$(MAKE) -C $(SRC_DIR) clean


install:
	$(MKDIR) $(INSTALL_DIR)
	$(CP) $(SRC_DIR)/$(TARGET) $(INSTALL_DIR)

uninstall:
	$(RM) $(INSTALL_DIR)/$(TARGET)
