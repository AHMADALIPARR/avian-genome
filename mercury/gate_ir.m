% AVIAN-MORPH typed quantum gate IR (Mercury)
% Typed circuit AST, gate rules, deterministic validation, OpenQASM emission.
% Compile with: mmc --make gate_ir

:- module gate_ir.
:- interface.
:- import_module io, list, string, float, int.

:- type qubit ---> q(int).
:- type angle ---> ang(float).
:- type bit_name == string.

:- type op
    --->    ry(angle, qubit)
    ;       rz(angle, qubit)
    ;       cx(qubit, qubit)
    ;       measure(qubit, bit_name).

:- type circuit
    --->    circuit(int, list(op), list(bit_name)).  % n_qubits, ops, bits

:- pred validate(circuit::in) is semidet.
:- pred to_qasm(circuit::in, string::out) is det.
:- func retinal_pair(float, float, float, float) = circuit.

:- implementation.

:- import_module require.

validate(circuit(N, Ops, Bits)) :-
    N > 0,
    list.foldl(validate_op(N), Ops, Bits, _).

:- pred validate_op(int::in, op::in, list(bit_name)::in, list(bit_name)::out) is semidet.
validate_op(N, ry(ang(_), q(I)), Bits, Bits) :-
    I >= 0, I < N.
validate_op(N, rz(ang(_), q(I)), Bits, Bits) :-
    I >= 0, I < N.
validate_op(N, cx(q(C), q(T)), Bits, Bits) :-
    C >= 0, C < N,
    T >= 0, T < N,
    C \= T.
validate_op(N, measure(q(I), B), Bits0, Bits) :-
    I >= 0, I < N,
    not list.member(B, Bits0),
    Bits = [B | Bits0].

to_qasm(circuit(N, Ops, Bits), Qasm) :-
    Header = "OPENQASM 3.0;\ninclude \"stdgates.inc\";\n" ++
              "qubit[" ++ int_to_string(N) ++ "] circuit;\n",
    BitDecls = string.join_list("", list.map(bit_decl, list.reverse(Bits))),
    Body = string.join_list("", list.map(op_line, Ops)),
    Qasm = Header ++ BitDecls ++ Body.

:- func bit_decl(bit_name) = string.
bit_decl(B) = "bit " ++ B ++ ";\n".

:- func op_line(op) = string.
op_line(ry(ang(A), q(I))) =
    "ry(" ++ float_to_string(A) ++ ") circuit[" ++ int_to_string(I) ++ "];\n".
op_line(rz(ang(A), q(I))) =
    "rz(" ++ float_to_string(A) ++ ") circuit[" ++ int_to_string(I) ++ "];\n".
op_line(cx(q(C), q(T))) =
    "cx circuit[" ++ int_to_string(C) ++ "], circuit[" ++ int_to_string(T) ++ "];\n".
op_line(measure(q(I), B)) =
    B ++ " = measure circuit[" ++ int_to_string(I) ++ "];\n".

retinal_pair(ThetaA, PhiA, ThetaB, PhiB) = circuit(6, Ops, ["spikeB", "spikeA"]) :-
    Ops = [
        ry(ang(ThetaA), q(0)),
        rz(ang(PhiA), q(1)),
        cx(q(0), q(1)),
        cx(q(1), q(2)),
        cx(q(2), q(3)),
        ry(ang(ThetaB), q(3)),
        rz(ang(PhiB), q(4)),
        cx(q(3), q(4)),
        cx(q(4), q(5)),
        measure(q(2), "spikeA"),
        measure(q(5), "spikeB")
    ].
