# morphic.rb — AVIAN-MORPH morphic neuron layer (Ruby).
#
# Prototype-based neurons: each neuron is cloned from a prototype,
# carries its own slots (kind, morphology, connectivity, state),
# and can delegate unread slots to its parent prototype. This is
the composition layer that the architecture assigns to Self;
# implemented here in Ruby so it runs on the machines that already
# run the orchestrator.
#
# No Self runtime is required. This is not a Self implementation;
# it is the morphic behaviour the pipeline needs, written in the
# language that already owns orchestration.

module Avian
  module Morphic
    # A prototype object with slots and optional parent delegation.
    class Prototype
      attr_reader :slots, :parent

      def initialize(slots = {}, parent: nil)
        @slots = slots.dup
        @parent = parent
      end

      def [](key)
        return @slots[key] if @slots.key?(key)
        return @parent[key] if @parent

        nil
      end

      def []=(key, value)
        @slots[key] = value
      end

      def clone_from(*extra)
        merged = @slots.merge(Hash[*extra])
        Prototype.new(merged, parent: self)
      end

      def kind
        self[:kind]
      end

      def name
        self[:name]
      end
    end

    # Base neuron prototype. Biological identity fields live here.
    NeuronProto = Prototype.new(
      kind: :neuron,
      morphology: :bipolar,
      connectivity: [],
      developmental_state: :mature,
      theta: 0.0,
      phi: 0.0
    )

    RetinalGanglion = NeuronProto.clone_from(
      :kind, :retinal,
      :name, 'retinal_ganglion',
      :morphology, :ganglion
    )

    Tectal = NeuronProto.clone_from(
      :kind, :tectal,
      :name, 'tectal',
      :morphology, :tectal_projection
    )

    # Concrete neuron instance with live angles and a spike label.
    class Neuron
      attr_reader :proto, :theta, :phi

      def initialize(proto, theta:, phi:)
        @proto = proto
        @theta = theta.to_f
        @phi = phi.to_f
      end

      def name
        @proto.name
      end

      def kind
        @proto.kind
      end

      def label(probability)
        return 'spiking' if probability >= 0.99
        return 'silent' if probability <= 0.01

        'graded'
      end

      def connect(other)
        list = (@proto[:connectivity] || []).dup
        list << other.name unless list.include?(other.name)
        @proto[:connectivity] = list
        self
      end
    end
  end
end
