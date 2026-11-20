# genome.rb — AVIAN-MORPH genomic geometry layer (Ruby).
#
# Maps genomic coordinates through the same 3D affine transform
# that the C11/CBQN/Vulkan geom kernels implement, then derives
# per-neuron circuit angles (theta, phi) from the transformed
# positions. The numerical transform itself is already verified
# in src/c11 and cbqn; this module owns the genome -> parameter
# policy for the retinal-tectal twin.
#
# Not q/kdb+. Runnable Ruby that uses the project's geometry
# contract so the orchestrator no longer hard-codes angles.

module Avian
  module Genome
    # Default affine map used by the first milestone.
    # Matrix is row-major 3x3; translation is 3-vector.
    # Matches the small fixture geometry in fixtures/arrays/geom_f32.txt
    # so the front-end genome table and the twin stay consistent.
    DEFAULT_MATRIX = [
      1.0, 0.5, 0.0,
      0.0, 2.0, -0.25,
      0.0, 0.0, 1.0
    ].freeze

    DEFAULT_TRANSLATION = [1.0, -2.0, 0.5].freeze

    # Four genomic points (x,y,z) that define the tissue layout.
    # P0, P1 used for neuron A (retinal); P2, P3 for neuron B (tectal).
    DEFAULT_POINTS = [
      [1.0, 2.0, 4.0],   # P0
      [-1.0, 0.0, 2.0],  # P1
      [0.0, 0.0, 0.0],   # P2
      [2.0, -4.0, 8.0]   # P3
    ].freeze

    class Layout
      attr_reader :matrix, :translation, :points, :transformed

      def initialize(matrix: DEFAULT_MATRIX, translation: DEFAULT_TRANSLATION, points: DEFAULT_POINTS)
        @matrix = matrix.dup.freeze
        @translation = translation.dup.freeze
        @points = points.map(&:dup).freeze
        @transformed = transform_all
      end

      # Apply the verified affine transform to one point.
      # out[r] = M[3r]*x + M[3r+1]*y + M[3r+2]*z + t[r]
      def transform_point(x, y, z)
        out = []
        3.times do |r|
          acc = 0.0
          acc = fma(@matrix[3 * r], x, acc)
          acc = fma(@matrix[3 * r + 1], y, acc)
          acc = fma(@matrix[3 * r + 2], z, acc)
          acc = fma(1.0, @translation[r], acc)
          out << acc
        end
        out
      end

      def transform_all
        @points.map { |p| transform_point(*p) }
      end

      # Derive (theta, phi) for a neuron from a pair of transformed points.
      # theta from the radial distance in the xy-plane (clamped to [0, pi]).
      # phi from the azimuthal angle.
      def angles_for(index_a, index_b = index_a)
        pa = @transformed[index_a]
        pb = @transformed[index_b]
        mx = (pa[0] + pb[0]) * 0.5
        my = (pa[1] + pb[1]) * 0.5
        mz = (pa[2] + pb[2]) * 0.5

        radial = Math.sqrt(mx * mx + my * my)
        theta = clamp(radial * 0.1, 0.0, Math::PI) # scale into [0, pi]
        phi = Math.atan2(my, mx)
        # Keep the first-milestone behaviour for the default layout:
        # retinal input is driven by stimulus, tectal rests at 0.
        # Callers override theta for the stimulus condition.
        [theta, phi, mz]
      end

      # Genome parameters for the retinal-tectal twin.
      # Stimulus still sets theta_a; the genome supplies the rest.
      def twin_params
        _theta_a, phi_a, _ = angles_for(0, 1)
        theta_b, phi_b, _ = angles_for(2, 3)
        {
          phi_a: 0.0,          # milestone: phase locked
          phi_b: 0.0,
          theta_b: 0.0,        # tectal at rest; CNOT copies retinal spike
          genome_theta_a: _theta_a,
          genome_phi_a: phi_a,
          genome_theta_b: theta_b,
          genome_phi_b: phi_b
        }
      end

      private

      def fma(a, b, c)
        # Match the C kernel's fused order as closely as Ruby allows.
        (a * b) + c
      end

      def clamp(v, lo, hi)
        return lo if v < lo
        return hi if v > hi

        v
      end
    end
  end
end
