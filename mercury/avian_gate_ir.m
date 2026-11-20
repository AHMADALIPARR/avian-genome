:- module avian_gate_ir.
:- interface.
:- import_module list, string.

:- type qubit ---> qubit(int).
:- type angle ---> angle(float).
:- type gate
    ---> h(qubit)
    ;    x(qubit)
    ;    rz(angle, qubit)
    ;    cx(qubit, qubit)
    ;    measure(qubit, int).
:- type circuit ---> circuit(int, int, list(gate)).
:- type check_result ---> valid ; invalid(string).

:- func check(circuit) = check_result.
:- func to_openqasm(circuit) = string.

:- implementation.
:- import_module int, float.

:- pred in_range(int::in, int::in) is semidet.
in_range(Size, Index) :- Index >= 0, Index < Size.

:- func gate_check(int, int, gate) = check_result.
gate_check(N, _, h(qubit(Q))) = (if in_range(N, Q) then valid else invalid("h: qubit out of range")).
gate_check(N, _, x(qubit(Q))) = (if in_range(N, Q) then valid else invalid("x: qubit out of range")).
gate_check(N, _, rz(angle(A), qubit(Q))) =
    (if in_range(N, Q), A = A, abs(A) < 1.0e300 then valid
     else invalid("rz: invalid angle or qubit")).
gate_check(N, _, cx(qubit(C), qubit(T))) =
    (if in_range(N, C), in_range(N, T), C \= T then valid
     else invalid("cx: invalid or identical qubits")).
gate_check(N, Bits, measure(qubit(Q), C)) =
    (if in_range(N, Q), in_range(Bits, C) then valid
     else invalid("measure: invalid index")).

:- func check_gates(int, int, list(gate)) = check_result.
check_gates(_, _, []) = valid.
check_gates(N, B, [G | Gs]) = Result :-
    First = gate_check(N, B, G),
    ( if First = valid then Result = check_gates(N, B, Gs)
      else Result = First ).

check(circuit(N, B, Gs)) = Result :-
    ( if N > 0, B >= 0 then Result = check_gates(N, B, Gs)
      else Result = invalid("register sizes must be positive/nonnegative") ).

:- func gate_qasm(gate) = string.
gate_qasm(h(qubit(Q))) = "h q[" ++ int_to_string(Q) ++ "];\n".
gate_qasm(x(qubit(Q))) = "x q[" ++ int_to_string(Q) ++ "];\n".
gate_qasm(rz(angle(A), qubit(Q))) =
    "rz(" ++ float_to_string(A) ++ ") q[" ++ int_to_string(Q) ++ "];\n".
gate_qasm(cx(qubit(C), qubit(T))) = "cx q[" ++ int_to_string(C) ++ "], q[" ++ int_to_string(T) ++ "];\n".
gate_qasm(measure(qubit(Q), C)) =
    "c[" ++ int_to_string(C) ++ "] = measure q[" ++ int_to_string(Q) ++ "];\n".

to_openqasm(C @ circuit(N, B, Gs)) = Text :-
    ( if check(C) = valid then
        Text = "OPENQASM 3.0;\ninclude \"stdgates.inc\";\n" ++
            "qubit[" ++ int_to_string(N) ++ "] q;\n" ++
            "bit[" ++ int_to_string(B) ++ "] c;\n" ++
            string.append_list(list.map(gate_qasm, Gs))
      else Text = "" ).
