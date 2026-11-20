"AVIAN-MORPH morphic neuron prototypes (Self).

Prototype-based neurons: identity, morphology, connectivity,
developmental state. Clone and delegate. This is the Self
layer source for the retinal-tectal twin.
"

traits neuron traits clonable

neuron _AddSlots: (|
  parent*           = traits neuron.
  name              = 'neuron'.
  kind              = 'generic'.
  morphology        = 'bipolar'.
  connectivity      = list copyRemoveAll.
  developmentalState = 'mature'.
  theta             = 0.0.
  phi               = 0.0.

  clone = (|
    | resend.clone _AddSlots: (|
        connectivity: connectivity copy.
      |)
  ).

  connect: other = (|
    | connectivity add: other name. self).

  labelProbability: p = (|
    | p >= 0.99 ifTrue: [^ 'spiking'].
      p <= 0.01 ifTrue: [^ 'silent'].
      'graded').
|)

retinalGanglion: neuron clone _AddSlots: (|
  name       = 'retinal_ganglion'.
  kind       = 'retinal'.
  morphology = 'ganglion'.
|)

tectal: neuron clone _AddSlots: (|
  name       = 'tectal'.
  kind       = 'tectal'.
  morphology = 'tectal_projection'.
|)

"Concrete twin: A drives B through CNOT coupling."
neuronA: retinalGanglion clone _AddSlots: (|
  theta = 0.0.
  phi   = 0.0.
|)

neuronB: tectal clone _AddSlots: (|
  theta = 0.0.
  phi   = 0.0.
|)

neuronA connect: neuronB.
