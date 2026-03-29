export APP_TITLE  := DSPICO8
export APP_AUTHOR := xXJSONDeruloXx
export V_MAJOR    := 0
export V_MINOR    := 2
export V_PATCH    := 0
export V_BUILD    := 0
export APP_VERSION := v$(V_MAJOR).$(V_MINOR).$(V_PATCH)

.PHONY: all nds nds-baseline tests rust-tests benchmarks framehashes rust-ffi-smoke rust-core-arm-check clean clean-nds clean-tests clean-rust

all: nds

nds:
	@$(MAKE) -C platform/nds-native

nds-baseline:
	@$(MAKE) -C platform/nds

tests:
	@$(MAKE) -C test
	cd test && ./testrunner.a

rust-tests:
	cargo test --manifest-path native-rs-core/Cargo.toml
	cargo test --manifest-path native-rs/Cargo.toml

benchmarks:
	./scripts/run-benchmarks.sh

framehashes:
	./scripts/compare-frame-hashes.sh

rust-ffi-smoke:
	./scripts/smoke-rust-ffi.sh

rust-core-arm-check:
	./scripts/check-rust-core-armv5te.sh

clean: clean-nds clean-tests clean-rust

clean-nds:
	@$(MAKE) -C platform/nds-native clean
	@$(MAKE) -C platform/nds clean

clean-tests:
	@$(MAKE) -C test clean

clean-rust:
	rm -rf native-rs/target native-rs-core/target
