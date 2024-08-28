.PHONY: all
all:
	@echo Executing Pre Build commands ...
	@export TORCS_BASE=/home/daniele/research/ongoing/championship2009/torcs-1.3.1/
	@export MAKE_DEFAULT=/home/daniele/research/ongoing/championship2009/torcs-1.3.1/Make-default.mk
	@echo Done
	@make -f "Makefile" install
