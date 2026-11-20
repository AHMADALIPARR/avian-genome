'Avian retinal-tectal morphogenesis: real Self objects, not Ruby facades.' printLine.

traits avianNeuron = (|
  parent* = traits clonable.
  initialize: identifier soma: position type: kind = (
    id: identifier.
    soma: position.
    neuronType: kind.
    developmentalStage: 0.
    dendrites: list copyRemoveAll.
    axons: list copyRemoveAll.
    self).
  addDendrite: point = (dendrites addLast: point. self).
  addAxonTarget: target = (axons addLast: target. self).
  advanceDevelopment = (developmentalStage: developmentalStage + 1. self).
  validate = (
    id isEmpty ifTrue: [error: 'neuron id must not be empty'].
    soma size = 3 ifFalse: [error: 'soma must contain x,y,z'].
    developmentalStage < 0 ifTrue: [error: 'negative developmental stage'].
    true).
|).

avianNeuron = (|
  parent* = traits avianNeuron.
  id <- ''.
  soma <- vector copySize: 3 FillingWith: 0.
  neuronType <- 'retinal'.
  developmentalStage <- 0.
  dendrites <- list copyRemoveAll.
  axons <- list copyRemoveAll.
|).

traits avianConnectome = (|
  parent* = traits clonable.
  add: neuron = (neuron validate. neurons at: neuron id Put: neuron. self).
  linkFrom: sourceId To: targetId = (
    (neurons includesKey: sourceId) ifFalse: [error: 'unknown source'].
    (neurons includesKey: targetId) ifFalse: [error: 'unknown target'].
    (neurons at: sourceId) addAxonTarget: targetId.
    self).
  advanceAll = (neurons do: [|:n| n advanceDevelopment]. self).
|).

avianConnectome = (|
  parent* = traits avianConnectome.
  neurons <- dictionary copyRemoveAll.
|).
