# gate_ir.rb — AVIAN-MORPH typed gate IR (Ruby).
#
# A small typed intermediate representation for the quantum circuits.
# Gates are constructed with explicit qubit indices and angle types;
# the IR validates arity, qubit bounds, and measurement targets before
# any OpenQASM is emitted. This is the role the architecture assigns
# to Mercury; implemented here so the pipeline can validate circuits
# without the Mercury compiler.
#
# Not Mercury source. Runnable typed builder that the orchestrator
# uses instead of raw string templates.

module Avian
  module GateIR
    class Error < StandardError; end

    class Qubit
      attr_reader :index
      def initialize(index)
        raise Error, "qubit index must be >= 0" if index < 0

        @index = index
      end

      def to_s
        "circuit[#{@index}]"
      end
    end

    class Angle
      attr_reader :radians
      def initialize(radians)
        @radians = radians.to_f
      end

      def to_s
        @radians.to_s
      end
    end

    # One gate or measurement in the circuit.
    class Op
      attr_reader :name, :qubits, :angle, :bit

      def initialize(name, qubits:, angle: nil, bit: nil)
        @name = name
        @qubits = Array(qubits)
        @angle = angle
        @bit = bit
      end
    end

    class Circuit
      attr_reader :n_qubits, :ops, :bits

      def initialize(n_qubits)
        raise Error, "n_qubits must be > 0" if n_qubits <= 0

        @n_qubits = n_qubits
        @ops = []
        @bits = []
      end

      def q(i)
        raise Error, "qubit #{i} out of range 0..#{@n_qubits - 1}" if i < 0 || i >= @n_qubits

        Qubit.new(i)
      end

      def ry(angle, target)
        a = Angle.new(angle)
        t = target.is_a?(Qubit) ? target : q(target)
        @ops << Op.new(:ry, qubits: [t], angle: a)
        self
      end

      def rz(angle, target)
        a = Angle.new(angle)
        t = target.is_a?(Qubit) ? target : q(target)
        @ops << Op.new(:rz, qubits: [t], angle: a)
        self
      end

      def cx(control, target)
        c = control.is_a?(Qubit) ? control : q(control)
        t = target.is_a?(Qubit) ? target : q(target)
        raise Error, "cx control and target must differ" if c.index == t.index

        @ops << Op.new(:cx, qubits: [c, t])
        self
      end

      def measure(target, bit_name)
        t = target.is_a?(Qubit) ? target : q(target)
        raise Error, "duplicate bit #{bit_name}" if @bits.include?(bit_name)

        @bits << bit_name
        @ops << Op.new(:measure, qubits: [t], bit: bit_name)
        self
      end

      def validate!
        @ops.each do |op|
          op.qubits.each do |qb|
            raise Error, "qubit #{qb.index} out of range" if qb.index >= @n_qubits
          end
          case op.name
          when :ry, :rz
            raise Error, "#{op.name} requires one qubit" unless op.qubits.size == 1
            raise Error, "#{op.name} requires an angle" if op.angle.nil?
          when :cx
            raise Error, 'cx requires two qubits' unless op.qubits.size == 2
          when :measure
            raise Error, 'measure requires one qubit' unless op.qubits.size == 1
            raise Error, 'measure requires a bit name' if op.bit.nil?
          else
            raise Error, "unknown op #{op.name}"
          end
        end
        self
      end

      # Emit OpenQASM 3 after validation.
      def to_qasm
        validate!
        lines = []
        lines << 'OPENQASM 3.0;'
        lines << 'include "stdgates.inc";'
        lines << "qubit[#{@n_qubits}] circuit;"
        @bits.each { |b| lines << "bit #{b};" }
        @ops.each do |op|
          case op.name
          when :ry
            lines << "ry(#{op.angle}) #{op.qubits[0]};"
          when :rz
            lines << "rz(#{op.angle}) #{op.qubits[0]};"
          when :cx
            lines << "cx #{op.qubits[0]}, #{op.qubits[1]};"
          when :measure
            lines << "#{op.bit} = measure #{op.qubits[0]};"
          end
        end
        lines.join("\n") + "\n"
      end
    end

    # Build the canonical 6-qubit retinal-tectal circuit.
    def self.retinal_pair(theta_a:, phi_a:, theta_b:, phi_b:)
      c = Circuit.new(6)
      # Neuron A (retinal ganglion) on qubits 0..2
      c.ry(theta_a, 0)
      c.rz(phi_a, 1)
      c.cx(0, 1)
      c.cx(1, 2)
      # Coupling: retinal output -> tectal input
      c.cx(2, 3)
      # Neuron B (tectal) on qubits 3..5
      c.ry(theta_b, 3)
      c.rz(phi_b, 4)
      c.cx(3, 4)
      c.cx(4, 5)
      c.measure(2, 'spikeA')
      c.measure(5, 'spikeB')
      c
    end
  end
end
