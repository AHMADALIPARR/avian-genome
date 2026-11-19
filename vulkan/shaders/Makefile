# Makefile — AVIAN-MORPH Vulkan shaders.
#
# Targets:
#   build  — compile each .comp shader to SPIR-V (.spv).
#   test   — compile and validate every shader with spirv-val.
#            Exits 0 only when every shader compiles and validates.
#   dis    — disassemble the SPIR-V to readable text (inspection only).
#   clean  — remove build artifacts (not the sources).
#
# The SPIR-V is validated, not executed: there is no GPU on this
# machine, so no shader is run and no CPU-vs-GPU comparison is made.
# See CONTRACT.md for the verification boundary.

VULKAN_BIN := $(HOME)/workspace/.tools/vulkan-env/bin
GLSLANG    := $(VULKAN_BIN)/glslangValidator
SPIRV_VAL  := $(VULKAN_BIN)/spirv-val
SPIRV_DIS  := $(VULKAN_BIN)/spirv-dis

SHADERS := scale_f32.comp scale_i32.comp conv1d_f32.comp reduce_segments.comp gather_scatter.comp geom_transform.comp complex_rotate.comp normalize.comp
SPV     := $(SHADERS:.comp=.spv)

.PHONY: build test dis clean

build: $(SPV)

%.spv: %.comp
	$(GLSLANG) -V -o $@ $<

test: build
	@for s in $(SPV); do \
		echo "validating $$s"; \
		$(SPIRV_VAL) $$s || exit 1; \
	done
	@echo "ALL SHADERS VALID"

dis: build
	@for s in $(SPV); do \
		$(SPIRV_DIS) $$s > $${s%.spv}.dis; \
	done
	@echo "disassembly written"

clean:
	rm -f $(SPV) *.dis
