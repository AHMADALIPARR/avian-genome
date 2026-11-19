# retinal_tectal.rb — AVIAN-MORPH Ruby orchestrator.
#
# The first milestone: a morphic digital twin of a small avian
# retinal-tectal circuit. This program is the orchestration layer.
# It defines the circuit, generates the concrete OpenQASM, drives
# the verified reference simulator, and reports the twin's state.
#
# The circuit is two neurons. Neuron A is a retinal ganglion cell.
# Neuron B is a tectal cell. The coupling is a controlled-NOT from
# A's output qubit to B's input qubit, representing the
# retinal-to-tectal projection. Each neuron keeps its own 3-qubit
# template; the coupling is a single entangling gate between them.
#
# Roles and constraints (see the architecture):
#   - Self holds the morphic prototypes. The Self toolchain is
#     not installed on this machine, so the neurons are plain
#     Ruby objects here, not Self prototypes. That substitution
#     is recorded, not hidden. See MILESTONE.md.
#   - Ruby holds the composition and orchestration. This file
#     is that layer.
#   - q holds the genomic geometry. q is commercial and not
#     installed here, so the genome is a plain parameter hash
#     in this file, not a q program. See MILESTONE.md.
#   - Mercury holds the typed gate validation. The Mercury
#     compiler is not installed here, so the templates are
#     validated by the reference simulator instead.
#   - The simulator (openqasm/sim/qsim_run) is the verified
#     component. Ruby drives it; it does not re-implement it.
#
# Usage:
#   ruby retinal_tectal.rb
#
# It runs three stimulus conditions (dark, dim, bright) through
# the circuit and prints the spike probability of each neuron
# for each condition, plus the generated OpenQASM for the bright
# condition. It exits 0 only when every simulator call succeeds
# and every reported probability is in [0, 1].

require 'open3'

REPO_ROOT = File.expand_path('..', __dir__)
QSIM_RUN = File.join(REPO_ROOT, 'openqasm', 'sim', 'qsim_run')
QASM_OUT = File.join(REPO_ROOT, 'fixtures', 'quantum', 'retinal_pair_bright.qasm')

# A neuron in the twin. kind is :retinal or :tectal. theta and
# phi are the gate angles in radians, set by the genome.
class Neuron
  attr_reader :name, :kind, :theta, :phi

  def initialize(name, kind, theta, phi)
    @name = name
    @kind = kind
    @theta = theta
    @phi = phi
  end

  def label(probability)
    return 'spiking' if probability >= 0.99
    return 'silent' if probability <= 0.01

    'graded'
  end
end

# The retinal-tectal circuit: neuron A (retinal ganglion) drives
# neuron B (tectal) through a controlled-NOT coupling.
class RetinalTectalCircuit
  attr_reader :neuron_a, :neuron_b

  def initialize(neuron_a, neuron_b)
    @neuron_a = neuron_a
    @neuron_b = neuron_b
  end

  # Generate the concrete OpenQASM for this circuit, with the
  # angles filled in. Returns the text.
  def to_qasm
    <<~QASM
      OPENQASM 3.0;
      include "stdgates.inc";
      qubit[6] circuit;
      bit spikeA;
      bit spikeB;
      ry(#{@neuron_a.theta}) circuit[0];
      rz(#{@neuron_a.phi}) circuit[1];
      cx circuit[0], circuit[1];
      cx circuit[1], circuit[2];
      cx circuit[2], circuit[3];
      ry(#{@neuron_b.theta}) circuit[3];
      rz(#{@neuron_b.phi}) circuit[4];
      cx circuit[3], circuit[4];
      cx circuit[4], circuit[5];
      spikeA = measure circuit[2];
      spikeB = measure circuit[5];
    QASM
  end

  # Run the circuit through the verified simulator and return
  # [P(spikeA), P(spikeB)]. Raises if the simulator fails.
  # Builds the simulator first if it is not present, so the
  # orchestrator works from a clean tree.
  def simulate
    ensure_simulator_built
    args = [
      QSIM_RUN, 'pair',
      @neuron_a.theta.to_s, @neuron_a.phi.to_s,
      @neuron_b.theta.to_s, @neuron_b.phi.to_s
    ]
    out, err, status = Open3.capture3(*args)
    raise "qsim_run failed: #{err}" unless status.success?

    probs = {}
    out.each_line do |line|
      key, value = line.split
      probs[key] = value.to_f
    end
    [probs.fetch('spikeA'), probs.fetch('spikeB')]
  end

  private

  def ensure_simulator_built
    return if File.executable?(QSIM_RUN)

    sim_dir = File.join(REPO_ROOT, 'openqasm', 'sim')
    _out, err, status = Open3.capture3('make', 'build', chdir: sim_dir)
    raise "could not build qsim_run: #{err}" unless status.success?
  end

  public
end

# The genome: the parameter set that defines the twin. In the
# full architecture this lives in q. Here it is a plain hash,
# because q is not installed on this machine.
GENOME = {
  phi_a: 0.0,
  phi_b: 0.0,
  theta_b: 0.0
}.freeze

# Stimulus conditions: the retinal input angle thetaA.
CONDITIONS = {
  'dark' => 0.0,
  'dim' => Math::PI / 2.0,
  'bright' => Math::PI
}.freeze

def main
  ok = true
  bright_qasm = nil

  CONDITIONS.each do |name, theta_a|
    neuron_a = Neuron.new('A', :retinal, theta_a, GENOME[:phi_a])
    neuron_b = Neuron.new('B', :tectal, GENOME[:theta_b], GENOME[:phi_b])
    circuit = RetinalTectalCircuit.new(neuron_a, neuron_b)
    bright_qasm = circuit.to_qasm if name == 'bright'

    begin
      pa, pb = circuit.simulate
    rescue StandardError => e
      puts "FAIL #{name}: #{e.message}"
      ok = false
      next
    end

    [pa, pb].each do |p|
      unless p >= 0.0 && p <= 1.0
        puts "FAIL #{name}: probability #{p} out of [0, 1]"
        ok = false
      end
    end

    puts "condition #{name} (thetaA=#{format('%.4f', theta_a)}):"
    puts "  neuron A (retinal ganglion): P(spike)=#{format('%.6f', pa)} #{neuron_a.label(pa)}"
    puts "  neuron B (tectal):          P(spike)=#{format('%.6f', pb)} #{neuron_b.label(pb)}"
  end

  File.write(QASM_OUT, bright_qasm)
  puts "wrote #{QASM_OUT}"

  if ok
    puts 'ALL CONDITIONS OK'
    0
  else
    puts 'ORCHESTRATION FAILED'
    1
  end
end

exit(main)
