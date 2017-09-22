#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := esp_download
EXTRA_COMPONENT_DIRS := $(shell pwd)/components

include $(IDF_PATH)/make/project.mk

