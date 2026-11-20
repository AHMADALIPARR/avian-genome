# avian-genome

C11 kernels, CBQN oracles, Vulkan shaders, OpenQASM + qsim, Ruby orchestrator, native Self/q/Mercury/Cryptol/SAW sources.

```
src/c11/          kernel ABI + tests + geom bridge
cbqn/             reference oracles
vulkan/shaders/   SPIR-V compute
vulkan/           GPU conformance host
openqasm/         circuits + C simulator
ruby/             morphic / genome / gate_ir / orchestrator
self/             Self morphology
q/                genomic geometry
mercury/          typed gate IR
cryptol/          kernel specs
saw/              equivalence scripts
fixtures/         array + quantum fixtures
```

```sh
make -C src/c11 test
make -C vulkan/shaders test
make -C openqasm/sim test
ruby ruby/retinal_tectal.rb
```
