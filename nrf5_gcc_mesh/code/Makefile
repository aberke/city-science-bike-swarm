PROJECT_NAME     := ble_app_beacon_swarm_s140
TARGETS          := nrf52840_xxaa
OUTPUT_DIRECTORY := _build
# VERBOSE := 1

SDK_ROOT := ..
PROJ_DIR := .

$(OUTPUT_DIRECTORY)/nrf52840_xxaa.out: \
  LINKER_SCRIPT  := ble_app_beacon_gcc_nrf52.ld

# Source files common to all targets
SRC_FILES += \
  $(PROJ_DIR)/src/main.c \
  $(PROJ_DIR)/mesh_provisionee.c \
  $(PROJ_DIR)/simple_hal.c \
  $(PROJ_DIR)/mesh_app_utils.c \
  $(PROJ_DIR)/mesh_adv.c \
  $(PROJ_DIR)/ble_softdevice_support.c \
  $(PROJ_DIR)/nrf_mesh_weak.c \
  $(PROJ_DIR)/app_error_weak.c \
  $(PROJ_DIR)/assertion_handler_weak.c \
  $(PROJ_DIR)/include/sdk_config.h \
  $(PROJ_DIR)/src/leds.c \
  $(PROJ_DIR)/led_softblink.c \
  $(PROJ_DIR)/system_nrf52840.c \
  $(SDK_ROOT)/mesh/core/src/internal_event.c \
  $(SDK_ROOT)/mesh/core/src/nrf_mesh_configure.c \
  $(SDK_ROOT)/mesh/core/src/aes.c \
  $(SDK_ROOT)/mesh/core/src/msg_cache.c \
  $(SDK_ROOT)/mesh/core/src/transport.c \
  $(SDK_ROOT)/mesh/core/src/event.c \
  $(SDK_ROOT)/mesh/core/src/packet_buffer.c \
  $(SDK_ROOT)/mesh/core/src/flash_manager_defrag.c \
  $(SDK_ROOT)/mesh/core/src/fifo.c \
  $(SDK_ROOT)/mesh/core/src/nrf_flash.c \
  $(SDK_ROOT)/mesh/core/src/net_state.c \
  $(SDK_ROOT)/mesh/core/src/mesh_flash.c \
  $(SDK_ROOT)/mesh/core/src/nrf_mesh_utils.c \
  $(SDK_ROOT)/mesh/core/src/nrf_mesh.c \
  $(SDK_ROOT)/mesh/core/src/queue.c \
  $(SDK_ROOT)/mesh/core/src/hal.c \
  $(SDK_ROOT)/mesh/core/src/aes_cmac.c \
  $(SDK_ROOT)/mesh/core/src/timer_scheduler.c \
  $(SDK_ROOT)/mesh/core/src/timer.c \
  $(SDK_ROOT)/mesh/core/src/long_timer.c \
  $(SDK_ROOT)/mesh/core/src/rand.c \
  $(SDK_ROOT)/mesh/core/src/nrf_mesh_opt.c \
  $(SDK_ROOT)/mesh/core/src/mesh_opt.c \
  $(SDK_ROOT)/mesh/core/src/timeslot.c \
  $(SDK_ROOT)/mesh/core/src/timeslot_timer.c \
  $(SDK_ROOT)/mesh/core/src/bearer_event.c \
  $(SDK_ROOT)/mesh/core/src/enc.c \
  $(SDK_ROOT)/mesh/core/src/network.c \
  $(SDK_ROOT)/mesh/core/src/net_packet.c \
  $(SDK_ROOT)/mesh/core/src/msqueue.c \
  $(SDK_ROOT)/mesh/core/src/nrf_mesh_keygen.c \
  $(SDK_ROOT)/mesh/core/src/cache.c \
  $(SDK_ROOT)/mesh/core/src/list.c \
  $(SDK_ROOT)/mesh/core/src/log.c \
  $(SDK_ROOT)/mesh/core/src/flash_manager.c \
  $(SDK_ROOT)/mesh/core/src/ccm_soft.c \
  $(SDK_ROOT)/mesh/core/src/toolchain.c \
  $(SDK_ROOT)/mesh/core/src/replay_cache.c \
  $(SDK_ROOT)/mesh/core/src/beacon.c \
  $(SDK_ROOT)/mesh/core/src/flash_manager_internal.c \
  $(SDK_ROOT)/mesh/core/src/core_tx.c \
  $(SDK_ROOT)/mesh/core/src/heartbeat.c \
  $(SDK_ROOT)/mesh/core/src/net_beacon.c \
  $(SDK_ROOT)/mesh/core/src/mesh_config.c \
  $(SDK_ROOT)/mesh/core/src/fsm.c \
  $(SDK_ROOT)/mesh/core/src/mesh_config_backend.c \
  $(SDK_ROOT)/mesh/core/src/mesh_config_flashman_glue.c \
  $(SDK_ROOT)/mesh/core/src/lpn.c \
  $(SDK_ROOT)/mesh/core/src/core_tx_lpn.c \
  $(SDK_ROOT)/mesh/core/src/mesh_lpn_subman.c \
  $(SDK_ROOT)/mesh/core/src/core_tx_local.c \
  $(SDK_ROOT)/mesh/core/src/core_tx_adv.c \
  $(SDK_ROOT)/mesh/core/src/mesh_mem_stdlib.c \
  $(SDK_ROOT)/mesh/stack/src/mesh_stack.c \
  $(SDK_ROOT)/mesh/access/src/access_publish.c \
  $(SDK_ROOT)/mesh/access/src/access_publish_retransmission.c \
  $(SDK_ROOT)/mesh/access/src/access.c \
  $(SDK_ROOT)/mesh/access/src/access_reliable.c \
  $(SDK_ROOT)/mesh/access/src/device_state_manager.c \
  $(SDK_ROOT)/mesh/bearer/src/ad_listener.c \
  $(SDK_ROOT)/mesh/bearer/src/ad_type_filter.c \
  $(SDK_ROOT)/mesh/bearer/src/adv_packet_filter.c \
  $(SDK_ROOT)/mesh/bearer/src/advertiser.c \
  $(SDK_ROOT)/mesh/bearer/src/bearer_handler.c \
  $(SDK_ROOT)/mesh/bearer/src/broadcast.c \
  $(SDK_ROOT)/mesh/bearer/src/filter_engine.c \
  $(SDK_ROOT)/mesh/bearer/src/gap_address_filter.c \
  $(SDK_ROOT)/mesh/bearer/src/radio_config.c \
  $(SDK_ROOT)/mesh/bearer/src/rssi_filter.c \
  $(SDK_ROOT)/mesh/bearer/src/scanner.c \
  $(SDK_ROOT)/mesh/bearer/src/mesh_pa_lna.c \
  $(SDK_ROOT)/external/rtt/src/SEGGER_RTT.c \
  $(SDK_ROOT)/external/rtt/src/SEGGER_RTT_printf.c \
  $(SDK_ROOT)/mesh/bootloader/src/uECC.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/ble/common/ble_advdata.c \
  $(SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
  $(SDK_ROOT)/components/ble/common/ble_conn_params.c \
  $(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
  $(SDK_ROOT)/components/ble/common/ble_srv_common.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_clock.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_pwm.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_timer.c \
  $(SDK_ROOT)/mesh/prov/src/prov_provisionee.c \
  $(SDK_ROOT)/mesh/prov/src/nrf_mesh_prov.c \
  $(SDK_ROOT)/mesh/prov/src/provisioning.c \
  $(SDK_ROOT)/mesh/prov/src/prov_beacon.c \
  $(SDK_ROOT)/mesh/prov/src/prov_utils.c \
  $(SDK_ROOT)/mesh/prov/src/prov_bearer_adv.c \
  $(SDK_ROOT)/models/foundation/config/src/composition_data.c \
  $(SDK_ROOT)/models/foundation/config/src/config_server.c \
  $(SDK_ROOT)/models/foundation/config/src/packed_index_list.c \
  $(SDK_ROOT)/models/foundation/health/src/health_server.c \
  $(SDK_ROOT)/external/app_timer/app_timer_mesh.c \
  $(SDK_ROOT)/external/app_timer/app_timer_workaround.c \

# Include folders common to all targets
INC_FOLDERS += \
  $(PROJ_DIR)/include \
  $(SDK_ROOT)/components/ble/common \
  $(SDK_ROOT)/components/softdevice/common \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/models/foundation/config/include \
  $(SDK_ROOT)/models/foundation/health/include \
  $(SDK_ROOT)/mesh/stack/api \
  $(SDK_ROOT)/mesh/core/api \
  $(SDK_ROOT)/mesh/core/include \
  $(SDK_ROOT)/mesh/access/api \
  $(SDK_ROOT)/mesh/access/include \
  $(SDK_ROOT)/mesh/dfu/api \
  $(SDK_ROOT)/mesh/dfu/include \
  $(SDK_ROOT)/mesh/prov/api \
  $(SDK_ROOT)/mesh/prov/include \
  $(SDK_ROOT)/mesh/bearer/api \
  $(SDK_ROOT)/mesh/bearer/include \
  $(SDK_ROOT)/mesh/gatt/api \
  $(SDK_ROOT)/mesh/gatt/include \
  $(SDK_ROOT)/mesh/friend/api \
  $(SDK_ROOT)/mesh/friend/include \
  $(SDK_ROOT)/components/softdevice/s140/headers/ \
  $(SDK_ROOT)/components/softdevice/s140/headers/nrf52/ \
  $(SDK_ROOT)/modules/nrfx \
  $(SDK_ROOT)/modules/nrfx/mdk \
  $(SDK_ROOT)/modules/nrfx/hal \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/toolchain/gcc \
  $(SDK_ROOT)/components/toolchain/cmsis/dsp/GCC \
  $(SDK_ROOT)/components/boards \
  $(SDK_ROOT)/integration/nrfx \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/libraries/delay \
  $(SDK_ROOT)/external/rtt/include \
  $(SDK_ROOT)/external/micro-ecc \
  $(SDK_ROOT)/mesh/core/include \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/integration/nrfx/legacy \

# Libraries common to all targets
LIB_FILES += \

# Optimization flags
OPT = -Og -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto
# C flags common to all targets

CFLAGS += $(OPT)
CFLAGS += -D__SIZEOF_WCHAR_T=4 -D__ARM_ARCH_7EM__ -D__SES_ARM -D__ARM_ARCH_FPV4_SP_D16__ -D__HEAP_SIZE__=1024 -D__SES_VERSION=45203 -D__GNU_LINKER -DNO_VTOR_CONFIG -DUSE_APP_CONFIG -DNRF52_SERIES -DNRF52840 -DNRF52840_XXAA -DS140 -DSOFTDEVICE_PRESENT -DNRF_SD_BLE_API_VERSION=7 -DBOARD_PCA10056 -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums
CFLAGS += -fmessage-length=0 -mlittle-endian -mfloat-abi=hard  -mfpu=fpv4-sp-d16 -mthumb -mtp=soft -munaligned-access -fno-dwarf2-cfi-asm -fno-common -std=c99 -g3 -gpubnames 
CFLAGS += -DuECC_OPTIMIZATION_LEVEL=2 -DuECC_SUPPORTS_secp160r1=0 -DuECC_SUPPORTS_secp192r1=0 -DuECC_SUPPORTS_secp224r1=0 -DuECC_SUPPORTS_secp256r1=1 -DuECC_SUPPORTS_secp256k1=0 -DuECC_SUPPORT_COMPRESSED_POINT=0
# C++ flags common to all targets
CXXFLAGS += $(OPT)
# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DAPP_TIMER_V2
ASMFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED
ASMFLAGS += -DBOARD_SWARM_NRF52840
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DNRF52840_XXAA
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=7
ASMFLAGS += -DS140
ASMFLAGS += -DSOFTDEVICE_PRESENT

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

nrf52840_xxaa: CFLAGS += -D__HEAP_SIZE=8192
nrf52840_xxaa: CFLAGS += -D__STACK_SIZE=8192
nrf52840_xxaa: ASMFLAGS += -D__HEAP_SIZE=8192
nrf52840_xxaa: ASMFLAGS += -D__STACK_SIZE=8192

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm


.PHONY: default help

# Default target - first one defined
default: nrf52840_xxaa

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo		nrf52840_xxaa
	@echo		flash_softdevice
	@echo		sdk_config - starting external tool for editing sdk_config.h
	@echo		flash      - flashing binary

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc


include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

.PHONY: flash flash_softdevice erase

# Flash the program
flash: default
	@echo Flashing: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
	./nrfjprog.sh --flash $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
	./nrfjprog.sh --reset

# Flash softdevice
flash_softdevice:
	@echo Flashing: s140_nrf52_7.0.1_softdevice.hex
	./nrfjprog.sh --flash-softdevice $(SDK_ROOT)/components/softdevice/s140/hex/s140_nrf52_7.0.1_softdevice.hex
	./nrfjprog.sh --reset

erase:
	nrfjprog -f nrf52 --eraseall

SDK_CONFIG_FILE := ../config/sdk_config.h
CMSIS_CONFIG_TOOL := $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)

dfu-package: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
	@echo Packaging $<
	adafruit-nrfutil dfu genpkg --sd-req 0xFFFE --dev-type 0x0052 --application $<  _build/dfu-package.zip

bootload: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex dfu-package
	@echo Flashing: $<
	nrfutil --verbose dfu serial --package _build/dfu-package.zip -p $(SERIAL_PORT) -b 115200 --singlebank --touch 1200
