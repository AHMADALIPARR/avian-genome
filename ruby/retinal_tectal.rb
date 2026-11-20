# retinal_tectal.rb — AVIAN-MORPH Ruby orchestrator.
#
# First milestone: morphic digital twin of a small avian
# retinal-tectal circuit. This file is the composition layer.
# It builds neurons from morphic prototypes, derives parameters
# from the genomic layout, validates the circuit through the
# typed gate IR, drives the verified reference simulator, and
# reports the twin's state.
#
# Usage:
#   ruby ruby/retinal_tectal.rb
#
# Exits 0 only when every simulator call succeeds and every
# reported probability is in [0, 1].

require 'open3'
require_relative 'morphic'
require_relative 'genome'
require_relative 'gate_ir'

REPO_ROOT = File.expand_path('..', __dir__)
QSIM_RUN = File.join(REPO_ROOT, 'openqasm', 'sim', 'qsim_run')
QASM_OUT = File.join(REPO_ROOT, 'fixtures', 'quantum', 'retinal_pair_bright.qasm')

class RetinalTectalCircuit
  attr_reader :neuron_a, :neuron_b, :ir

  def initialize(neuron_a, neuron_b)
    @neuron_a = neuron_a
    @neuron_b = neuron_b
    @neuron_a.connect(@neuron_b)
    @ir = Avian::GateIR.retinal_pair(
      theta_a: @neuron_a.theta,
      phi_a: @neuron_a.phi,
      theta_b: @neuron_b.theta,
      phi_b: @neuron_b.phi
    )
  end

  def to_qasm
    @ir.to_qasm
  end

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
end

# Stimulus conditions: the retinal input angle thetaA.
CONDITIONS = {
  'dark' => 0.0,
  'dim' => Math::PI / 2.0,
  'bright' => Math::PI
}.freeze

def main
  layout = Avian::Genome::Layout.new
  params = layout.twin_params
  ok = true
  bright_qasm = nil

  CONDITIONS.each do |name, theta_a|
    proto_a = Avian::Morphic::RetinalGanglion
    proto_b = Avian::Morphic::Tectal
    neuron_a = Avian::Morphic::Neuron.new(proto_a, theta: theta_a, phi: params[:phi_a])
    neuron_b = Avian::Morphic::Neuron.new(proto_b, theta: params[:theta_b], phi: params[:phi_b])
    circuit = RetinalTectalCircuit.new(neuron_a, neuron_b)
    bright_qasm = circuit.to_qasm if name == 'bright'

    begin
      # Typed IR validates before any simulation.
      circuit.ir.validate!
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
    puts "  neuron A (#{neuron_a.name}): P(spike)=#{format('%.6f', pa)} #{neuron_a.label(pa)}"
    puts "  neuron B (#{neuron_b.name}): P(spike)=#{format('%.6f', pb)} #{neuron_b.label(pb)}"
  end

  File.write(QASM_OUT, bright_qasm) if bright_qasm
  puts "wrote #{QASM_OUT}" if bright_qasm

  if ok
    puts 'ALL CONDITIONS OK'
    0
  else
    puts 'ORCHESTRATION FAILED'
    1
  end
end

exit(main)
