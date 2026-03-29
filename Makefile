export APP_TITLE  := DSPICO8
export APP_AUTHOR := xXJSONDeruloXx
export V_MAJOR    := 0
export V_MINOR    := 2
export V_PATCH    := 0
export V_BUILD    := 0
export APP_VERSION := v$(V_MAJOR).$(V_MINOR).$(V_PATCH)

.PHONY: all nds nds-baseline tests benchmarks clean clean-nds clean-tests

all: nds

nds:
	@$(MAKE) -C platform/nds-native

nds-baseline:
	@$(MAKE) -C platform/nds

tests:
	@$(MAKE) -C test
	cd test && ./testrunner.a

benchmarks:
	./scripts/run-benchmarks.sh

clean: clean-nds clean-tests

clean-nds:
	@$(MAKE) -C platform/nds-native clean
	@$(MAKE) -C platform/nds clean

clean-tests:
	@$(MAKE) -C test clean
