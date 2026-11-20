:- module avian_gate_main.
:- interface.
:- import_module io.
:- pred main(io::di, io::uo) is det.
:- implementation.
:- import_module avian_gate_ir, list, string.
main(!IO) :-
    C = circuit(2, 2, [h(qubit(0)), cx(qubit(0), qubit(1)),
                         rz(angle(0.7853981633974483), qubit(1)),
                         measure(qubit(0), 0), measure(qubit(1), 1)]),
    ( if check(C) = valid then io.write_string(to_openqasm(C), !IO)
      else io.write_string("invalid circuit\n", !IO) ).
