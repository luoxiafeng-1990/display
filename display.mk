################################################################################
#
# display - Display Framework Package
#
################################################################################

DISPLAY_VERSION = 1.0
DISPLAY_SITE = ../packages/display
DISPLAY_SITE_METHOD = local
DISPLAY_INSTALL_STAGING = YES
DISPLAY_INSTALL_TARGET = YES
DISPLAY_AUTORECONF = YES
DISPLAY_DEPENDENCIES = host-autoconf host-automake host-libtool liburing

ifeq ($(BR2_ENABLE_DEBUG),y)
DISPLAY_CONF_OPTS += --enable-debug
endif

define DISPLAY_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/display_test $(TARGET_DIR)/usr/local/bin
endef

$(eval $(autotools-package))

