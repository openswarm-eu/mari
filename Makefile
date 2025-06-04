DOCKER_IMAGE ?= aabadie/dotbot:latest
DOCKER_TARGETS ?= all
PACKAGES_DIR_OPT ?=
SEGGER_DIR ?= /opt/segger
BUILD_CONFIG ?= Debug

.PHONY: gateway node clean-gateway clean-node clean distclean docker

all: gateway node

node:
	@echo "\e[1mBuilding $@ application\e[0m"
	"$(SEGGER_DIR)/bin/emBuild" mira-node-nrf52840dk.emProject -project 03app_$@ -config $(BUILD_CONFIG) $(PACKAGES_DIR_OPT) -rebuild -verbose
	@echo "\e[1mDone\e[0m\n"

gateway:
	@echo "\e[1mBuilding $@ application\e[0m"
	"$(SEGGER_DIR)/bin/emBuild" mira-gateway-nrf52840dk.emProject -project 03app_$@ -config $(BUILD_CONFIG) $(PACKAGES_DIR_OPT) -rebuild -verbose
	@echo "\e[1mDone\e[0m\n"

clean-gateway:
	"$(SEGGER_DIR)/bin/emBuild" mira-gateway-nrf52840dk.emProject -config $(BUILD_CONFIG) -clean

clean-node:
	"$(SEGGER_DIR)/bin/emBuild" mira-node-nrf52840dk.emProject -config $(BUILD_CONFIG) -clean

clean: clean-gateway clean-node

distclean: clean

docker:
	docker run --rm -i \
		-e BUILD_CONFIG="$(BUILD_CONFIG)" \
		-e PACKAGES_DIR_OPT="-packagesdir $(SEGGER_DIR)/packages" \
		-e SEGGER_DIR="$(SEGGER_DIR)" \
		-v $(PWD):/dotbot $(DOCKER_IMAGE) \
		make $(DOCKER_TARGETS)
