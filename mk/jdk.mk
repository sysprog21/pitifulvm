JAVA = java
JAVA := $(shell which $(JAVA))
ifndef JAVA
$(error "java is required.")
endif

JAVAC = javac
JAVAC := $(shell which $(JAVAC))
ifndef JAVAC
$(error "javac is required.")
endif
