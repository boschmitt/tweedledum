OPENQASM 2.0;
include "qelib1.inc";
qreg q[13];
h q[2];
t q[8];
t q[10];
t q[2];
cx q[10], q[8];
cx q[2], q[10];
cx q[8], q[2];
tdg q[10];
cx q[8], q[10];
tdg q[8];
tdg q[10];
t q[2];
cx q[2], q[10];
cx q[8], q[2];
cx q[10], q[8];
h q[2];
h q[10];
t q[12];
t q[11];
t q[10];
cx q[11], q[12];
cx q[10], q[11];
cx q[12], q[10];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[10];
cx q[10], q[11];
cx q[12], q[10];
cx q[11], q[12];
h q[10];
h q[2];
t q[8];
t q[10];
t q[2];
cx q[10], q[8];
cx q[2], q[10];
cx q[8], q[2];
tdg q[10];
cx q[8], q[10];
tdg q[8];
tdg q[10];
t q[2];
cx q[2], q[10];
cx q[8], q[2];
cx q[10], q[8];
h q[2];
h q[10];
t q[12];
t q[11];
t q[10];
cx q[11], q[12];
cx q[10], q[11];
cx q[12], q[10];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[10];
cx q[10], q[11];
cx q[12], q[10];
cx q[11], q[12];
h q[10];
h q[1];
t q[7];
t q[12];
t q[1];
cx q[12], q[7];
cx q[1], q[12];
cx q[7], q[1];
tdg q[12];
cx q[7], q[12];
tdg q[7];
tdg q[12];
t q[1];
cx q[1], q[12];
cx q[7], q[1];
cx q[12], q[7];
h q[1];
h q[12];
t q[11];
t q[10];
t q[12];
cx q[10], q[11];
cx q[12], q[10];
cx q[11], q[12];
tdg q[10];
cx q[11], q[10];
tdg q[11];
tdg q[10];
t q[12];
cx q[12], q[10];
cx q[11], q[12];
cx q[10], q[11];
h q[12];
h q[1];
t q[7];
t q[12];
t q[1];
cx q[12], q[7];
cx q[1], q[12];
cx q[7], q[1];
tdg q[12];
cx q[7], q[12];
tdg q[7];
tdg q[12];
t q[1];
cx q[1], q[12];
cx q[7], q[1];
cx q[12], q[7];
h q[1];
h q[12];
t q[11];
t q[10];
t q[12];
cx q[10], q[11];
cx q[12], q[10];
cx q[11], q[12];
tdg q[10];
cx q[11], q[10];
tdg q[11];
tdg q[10];
t q[12];
cx q[12], q[10];
cx q[11], q[12];
cx q[10], q[11];
h q[12];
h q[3];
t q[11];
t q[7];
t q[3];
cx q[7], q[11];
cx q[3], q[7];
cx q[11], q[3];
tdg q[7];
cx q[11], q[7];
tdg q[11];
tdg q[7];
t q[3];
cx q[3], q[7];
cx q[11], q[3];
cx q[7], q[11];
h q[3];
h q[2];
t q[11];
t q[7];
t q[2];
cx q[7], q[11];
cx q[2], q[7];
cx q[11], q[2];
tdg q[7];
cx q[11], q[7];
tdg q[11];
tdg q[7];
t q[2];
cx q[2], q[7];
cx q[11], q[2];
cx q[7], q[11];
h q[2];
h q[2];
t q[10];
t q[6];
t q[2];
cx q[6], q[10];
cx q[2], q[6];
cx q[10], q[2];
tdg q[6];
cx q[10], q[6];
tdg q[10];
tdg q[6];
t q[2];
cx q[2], q[6];
cx q[10], q[2];
cx q[6], q[10];
h q[2];
h q[1];
t q[10];
t q[6];
t q[1];
cx q[6], q[10];
cx q[1], q[6];
cx q[10], q[1];
tdg q[6];
cx q[10], q[6];
tdg q[10];
tdg q[6];
t q[1];
cx q[1], q[6];
cx q[10], q[1];
cx q[6], q[10];
h q[1];
h q[0];
t q[6];
t q[12];
t q[0];
cx q[12], q[6];
cx q[0], q[12];
cx q[6], q[0];
tdg q[12];
cx q[6], q[12];
tdg q[6];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[6], q[0];
cx q[12], q[6];
h q[0];
h q[12];
t q[10];
t q[9];
t q[12];
cx q[9], q[10];
cx q[12], q[9];
cx q[10], q[12];
tdg q[9];
cx q[10], q[9];
tdg q[10];
tdg q[9];
t q[12];
cx q[12], q[9];
cx q[10], q[12];
cx q[9], q[10];
h q[12];
h q[0];
t q[6];
t q[12];
t q[0];
cx q[12], q[6];
cx q[0], q[12];
cx q[6], q[0];
tdg q[12];
cx q[6], q[12];
tdg q[6];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[6], q[0];
cx q[12], q[6];
h q[0];
h q[12];
t q[10];
t q[9];
t q[12];
cx q[9], q[10];
cx q[12], q[9];
cx q[10], q[12];
tdg q[9];
cx q[10], q[9];
tdg q[10];
tdg q[9];
t q[12];
cx q[12], q[9];
cx q[10], q[12];
cx q[9], q[10];
h q[12];
h q[1];
t q[9];
t q[5];
t q[1];
cx q[5], q[9];
cx q[1], q[5];
cx q[9], q[1];
tdg q[5];
cx q[9], q[5];
tdg q[9];
tdg q[5];
t q[1];
cx q[1], q[5];
cx q[9], q[1];
cx q[5], q[9];
h q[1];
h q[0];
t q[9];
t q[5];
t q[0];
cx q[5], q[9];
cx q[0], q[5];
cx q[9], q[0];
tdg q[5];
cx q[9], q[5];
tdg q[9];
tdg q[5];
t q[0];
cx q[0], q[5];
cx q[9], q[0];
cx q[5], q[9];
h q[0];
h q[0];
t q[5];
t q[12];
t q[0];
cx q[12], q[5];
cx q[0], q[12];
cx q[5], q[0];
tdg q[12];
cx q[5], q[12];
tdg q[5];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[5], q[0];
cx q[12], q[5];
h q[0];
h q[12];
t q[10];
t q[6];
t q[12];
cx q[6], q[10];
cx q[12], q[6];
cx q[10], q[12];
tdg q[6];
cx q[10], q[6];
tdg q[10];
tdg q[6];
t q[12];
cx q[12], q[6];
cx q[10], q[12];
cx q[6], q[10];
h q[12];
h q[0];
t q[5];
t q[12];
t q[0];
cx q[12], q[5];
cx q[0], q[12];
cx q[5], q[0];
tdg q[12];
cx q[5], q[12];
tdg q[5];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[5], q[0];
cx q[12], q[5];
h q[0];
h q[12];
t q[10];
t q[6];
t q[12];
cx q[6], q[10];
cx q[12], q[6];
cx q[10], q[12];
tdg q[6];
cx q[10], q[6];
tdg q[10];
tdg q[6];
t q[12];
cx q[12], q[6];
cx q[10], q[12];
cx q[6], q[10];
h q[12];
x q[5];
h q[0];
t q[5];
t q[12];
t q[0];
cx q[12], q[5];
cx q[0], q[12];
cx q[5], q[0];
tdg q[12];
cx q[5], q[12];
tdg q[5];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[5], q[0];
cx q[12], q[5];
h q[0];
h q[12];
t q[7];
t q[9];
t q[12];
cx q[9], q[7];
cx q[12], q[9];
cx q[7], q[12];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[12];
cx q[12], q[9];
cx q[7], q[12];
cx q[9], q[7];
h q[12];
h q[9];
t q[11];
t q[10];
t q[9];
cx q[10], q[11];
cx q[9], q[10];
cx q[11], q[9];
tdg q[10];
cx q[11], q[10];
tdg q[11];
tdg q[10];
t q[9];
cx q[9], q[10];
cx q[11], q[9];
cx q[10], q[11];
h q[9];
h q[12];
t q[7];
t q[9];
t q[12];
cx q[9], q[7];
cx q[12], q[9];
cx q[7], q[12];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[12];
cx q[12], q[9];
cx q[7], q[12];
cx q[9], q[7];
h q[12];
h q[0];
t q[5];
t q[12];
t q[0];
cx q[12], q[5];
cx q[0], q[12];
cx q[5], q[0];
tdg q[12];
cx q[5], q[12];
tdg q[5];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[5], q[0];
cx q[12], q[5];
h q[0];
h q[12];
t q[7];
t q[9];
t q[12];
cx q[9], q[7];
cx q[12], q[9];
cx q[7], q[12];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[12];
cx q[12], q[9];
cx q[7], q[12];
cx q[9], q[7];
h q[12];
h q[9];
t q[11];
t q[10];
t q[9];
cx q[10], q[11];
cx q[9], q[10];
cx q[11], q[9];
tdg q[10];
cx q[11], q[10];
tdg q[11];
tdg q[10];
t q[9];
cx q[9], q[10];
cx q[11], q[9];
cx q[10], q[11];
h q[9];
h q[12];
t q[7];
t q[9];
t q[12];
cx q[9], q[7];
cx q[12], q[9];
cx q[7], q[12];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[12];
cx q[12], q[9];
cx q[7], q[12];
cx q[9], q[7];
h q[12];
h q[0];
t q[5];
t q[12];
t q[0];
cx q[12], q[5];
cx q[0], q[12];
cx q[5], q[0];
tdg q[12];
cx q[5], q[12];
tdg q[5];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[5], q[0];
cx q[12], q[5];
h q[0];
h q[12];
t q[6];
t q[10];
t q[12];
cx q[10], q[6];
cx q[12], q[10];
cx q[6], q[12];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[12];
cx q[12], q[10];
cx q[6], q[12];
cx q[10], q[6];
h q[12];
h q[10];
t q[11];
t q[7];
t q[10];
cx q[7], q[11];
cx q[10], q[7];
cx q[11], q[10];
tdg q[7];
cx q[11], q[7];
tdg q[11];
tdg q[7];
t q[10];
cx q[10], q[7];
cx q[11], q[10];
cx q[7], q[11];
h q[10];
h q[12];
t q[6];
t q[10];
t q[12];
cx q[10], q[6];
cx q[12], q[10];
cx q[6], q[12];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[12];
cx q[12], q[10];
cx q[6], q[12];
cx q[10], q[6];
h q[12];
h q[0];
t q[5];
t q[12];
t q[0];
cx q[12], q[5];
cx q[0], q[12];
cx q[5], q[0];
tdg q[12];
cx q[5], q[12];
tdg q[5];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[5], q[0];
cx q[12], q[5];
h q[0];
h q[12];
t q[6];
t q[10];
t q[12];
cx q[10], q[6];
cx q[12], q[10];
cx q[6], q[12];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[12];
cx q[12], q[10];
cx q[6], q[12];
cx q[10], q[6];
h q[12];
h q[10];
t q[11];
t q[7];
t q[10];
cx q[7], q[11];
cx q[10], q[7];
cx q[11], q[10];
tdg q[7];
cx q[11], q[7];
tdg q[11];
tdg q[7];
t q[10];
cx q[10], q[7];
cx q[11], q[10];
cx q[7], q[11];
h q[10];
h q[12];
t q[6];
t q[10];
t q[12];
cx q[10], q[6];
cx q[12], q[10];
cx q[6], q[12];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[12];
cx q[12], q[10];
cx q[6], q[12];
cx q[10], q[6];
h q[12];
x q[9];
h q[0];
t q[6];
t q[12];
t q[0];
cx q[12], q[6];
cx q[0], q[12];
cx q[6], q[0];
tdg q[12];
cx q[6], q[12];
tdg q[6];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[6], q[0];
cx q[12], q[6];
h q[0];
h q[12];
t q[7];
t q[10];
t q[12];
cx q[10], q[7];
cx q[12], q[10];
cx q[7], q[12];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[12];
cx q[12], q[10];
cx q[7], q[12];
cx q[10], q[7];
h q[12];
h q[10];
t q[11];
t q[9];
t q[10];
cx q[9], q[11];
cx q[10], q[9];
cx q[11], q[10];
tdg q[9];
cx q[11], q[9];
tdg q[11];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[11], q[10];
cx q[9], q[11];
h q[10];
h q[12];
t q[7];
t q[10];
t q[12];
cx q[10], q[7];
cx q[12], q[10];
cx q[7], q[12];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[12];
cx q[12], q[10];
cx q[7], q[12];
cx q[10], q[7];
h q[12];
h q[0];
t q[6];
t q[12];
t q[0];
cx q[12], q[6];
cx q[0], q[12];
cx q[6], q[0];
tdg q[12];
cx q[6], q[12];
tdg q[6];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[6], q[0];
cx q[12], q[6];
h q[0];
h q[12];
t q[7];
t q[10];
t q[12];
cx q[10], q[7];
cx q[12], q[10];
cx q[7], q[12];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[12];
cx q[12], q[10];
cx q[7], q[12];
cx q[10], q[7];
h q[12];
h q[10];
t q[11];
t q[9];
t q[10];
cx q[9], q[11];
cx q[10], q[9];
cx q[11], q[10];
tdg q[9];
cx q[11], q[9];
tdg q[11];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[11], q[10];
cx q[9], q[11];
h q[10];
h q[12];
t q[7];
t q[10];
t q[12];
cx q[10], q[7];
cx q[12], q[10];
cx q[7], q[12];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[12];
cx q[12], q[10];
cx q[7], q[12];
cx q[10], q[7];
h q[12];
h q[0];
t q[7];
t q[12];
t q[0];
cx q[12], q[7];
cx q[0], q[12];
cx q[7], q[0];
tdg q[12];
cx q[7], q[12];
tdg q[7];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[7], q[0];
cx q[12], q[7];
h q[0];
h q[12];
t q[9];
t q[8];
t q[12];
cx q[8], q[9];
cx q[12], q[8];
cx q[9], q[12];
tdg q[8];
cx q[9], q[8];
tdg q[9];
tdg q[8];
t q[12];
cx q[12], q[8];
cx q[9], q[12];
cx q[8], q[9];
h q[12];
h q[8];
t q[11];
t q[10];
t q[8];
cx q[10], q[11];
cx q[8], q[10];
cx q[11], q[8];
tdg q[10];
cx q[11], q[10];
tdg q[11];
tdg q[10];
t q[8];
cx q[8], q[10];
cx q[11], q[8];
cx q[10], q[11];
h q[8];
h q[12];
t q[9];
t q[8];
t q[12];
cx q[8], q[9];
cx q[12], q[8];
cx q[9], q[12];
tdg q[8];
cx q[9], q[8];
tdg q[9];
tdg q[8];
t q[12];
cx q[12], q[8];
cx q[9], q[12];
cx q[8], q[9];
h q[12];
h q[0];
t q[7];
t q[12];
t q[0];
cx q[12], q[7];
cx q[0], q[12];
cx q[7], q[0];
tdg q[12];
cx q[7], q[12];
tdg q[7];
tdg q[12];
t q[0];
cx q[0], q[12];
cx q[7], q[0];
cx q[12], q[7];
h q[0];
h q[12];
t q[9];
t q[8];
t q[12];
cx q[8], q[9];
cx q[12], q[8];
cx q[9], q[12];
tdg q[8];
cx q[9], q[8];
tdg q[9];
tdg q[8];
t q[12];
cx q[12], q[8];
cx q[9], q[12];
cx q[8], q[9];
h q[12];
h q[8];
t q[11];
t q[10];
t q[8];
cx q[10], q[11];
cx q[8], q[10];
cx q[11], q[8];
tdg q[10];
cx q[11], q[10];
tdg q[11];
tdg q[10];
t q[8];
cx q[8], q[10];
cx q[11], q[8];
cx q[10], q[11];
h q[8];
h q[12];
t q[9];
t q[8];
t q[12];
cx q[8], q[9];
cx q[12], q[8];
cx q[9], q[12];
tdg q[8];
cx q[9], q[8];
tdg q[9];
tdg q[8];
t q[12];
cx q[12], q[8];
cx q[9], q[12];
cx q[8], q[9];
h q[12];
h q[1];
t q[9];
t q[5];
t q[1];
cx q[5], q[9];
cx q[1], q[5];
cx q[9], q[1];
tdg q[5];
cx q[9], q[5];
tdg q[9];
tdg q[5];
t q[1];
cx q[1], q[5];
cx q[9], q[1];
cx q[5], q[9];
h q[1];
h q[1];
t q[6];
t q[10];
t q[1];
cx q[10], q[6];
cx q[1], q[10];
cx q[6], q[1];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[1];
cx q[1], q[10];
cx q[6], q[1];
cx q[10], q[6];
h q[1];
h q[10];
t q[8];
t q[9];
t q[10];
cx q[9], q[8];
cx q[10], q[9];
cx q[8], q[10];
tdg q[9];
cx q[8], q[9];
tdg q[8];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[8], q[10];
cx q[9], q[8];
h q[10];
h q[9];
t q[12];
t q[11];
t q[9];
cx q[11], q[12];
cx q[9], q[11];
cx q[12], q[9];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[9];
cx q[9], q[11];
cx q[12], q[9];
cx q[11], q[12];
h q[9];
h q[10];
t q[8];
t q[9];
t q[10];
cx q[9], q[8];
cx q[10], q[9];
cx q[8], q[10];
tdg q[9];
cx q[8], q[9];
tdg q[8];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[8], q[10];
cx q[9], q[8];
h q[10];
h q[1];
t q[6];
t q[10];
t q[1];
cx q[10], q[6];
cx q[1], q[10];
cx q[6], q[1];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[1];
cx q[1], q[10];
cx q[6], q[1];
cx q[10], q[6];
h q[1];
h q[10];
t q[8];
t q[9];
t q[10];
cx q[9], q[8];
cx q[10], q[9];
cx q[8], q[10];
tdg q[9];
cx q[8], q[9];
tdg q[8];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[8], q[10];
cx q[9], q[8];
h q[10];
h q[9];
t q[12];
t q[11];
t q[9];
cx q[11], q[12];
cx q[9], q[11];
cx q[12], q[9];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[9];
cx q[9], q[11];
cx q[12], q[9];
cx q[11], q[12];
h q[9];
h q[10];
t q[8];
t q[9];
t q[10];
cx q[9], q[8];
cx q[10], q[9];
cx q[8], q[10];
tdg q[9];
cx q[8], q[9];
tdg q[8];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[8], q[10];
cx q[9], q[8];
h q[10];
h q[1];
t q[6];
t q[12];
t q[1];
cx q[12], q[6];
cx q[1], q[12];
cx q[6], q[1];
tdg q[12];
cx q[6], q[12];
tdg q[6];
tdg q[12];
t q[1];
cx q[1], q[12];
cx q[6], q[1];
cx q[12], q[6];
h q[1];
h q[12];
t q[11];
t q[7];
t q[12];
cx q[7], q[11];
cx q[12], q[7];
cx q[11], q[12];
tdg q[7];
cx q[11], q[7];
tdg q[11];
tdg q[7];
t q[12];
cx q[12], q[7];
cx q[11], q[12];
cx q[7], q[11];
h q[12];
h q[1];
t q[6];
t q[12];
t q[1];
cx q[12], q[6];
cx q[1], q[12];
cx q[6], q[1];
tdg q[12];
cx q[6], q[12];
tdg q[6];
tdg q[12];
t q[1];
cx q[1], q[12];
cx q[6], q[1];
cx q[12], q[6];
h q[1];
h q[12];
t q[11];
t q[7];
t q[12];
cx q[7], q[11];
cx q[12], q[7];
cx q[11], q[12];
tdg q[7];
cx q[11], q[7];
tdg q[11];
tdg q[7];
t q[12];
cx q[12], q[7];
cx q[11], q[12];
cx q[7], q[11];
h q[12];
x q[6];
h q[1];
t q[6];
t q[11];
t q[1];
cx q[11], q[6];
cx q[1], q[11];
cx q[6], q[1];
tdg q[11];
cx q[6], q[11];
tdg q[6];
tdg q[11];
t q[1];
cx q[1], q[11];
cx q[6], q[1];
cx q[11], q[6];
h q[1];
h q[11];
t q[7];
t q[10];
t q[11];
cx q[10], q[7];
cx q[11], q[10];
cx q[7], q[11];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[7], q[11];
cx q[10], q[7];
h q[11];
h q[10];
t q[12];
t q[8];
t q[10];
cx q[8], q[12];
cx q[10], q[8];
cx q[12], q[10];
tdg q[8];
cx q[12], q[8];
tdg q[12];
tdg q[8];
t q[10];
cx q[10], q[8];
cx q[12], q[10];
cx q[8], q[12];
h q[10];
h q[11];
t q[7];
t q[10];
t q[11];
cx q[10], q[7];
cx q[11], q[10];
cx q[7], q[11];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[7], q[11];
cx q[10], q[7];
h q[11];
h q[1];
t q[6];
t q[11];
t q[1];
cx q[11], q[6];
cx q[1], q[11];
cx q[6], q[1];
tdg q[11];
cx q[6], q[11];
tdg q[6];
tdg q[11];
t q[1];
cx q[1], q[11];
cx q[6], q[1];
cx q[11], q[6];
h q[1];
h q[11];
t q[7];
t q[10];
t q[11];
cx q[10], q[7];
cx q[11], q[10];
cx q[7], q[11];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[7], q[11];
cx q[10], q[7];
h q[11];
h q[10];
t q[12];
t q[8];
t q[10];
cx q[8], q[12];
cx q[10], q[8];
cx q[12], q[10];
tdg q[8];
cx q[12], q[8];
tdg q[12];
tdg q[8];
t q[10];
cx q[10], q[8];
cx q[12], q[10];
cx q[8], q[12];
h q[10];
h q[11];
t q[7];
t q[10];
t q[11];
cx q[10], q[7];
cx q[11], q[10];
cx q[7], q[11];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[7], q[11];
cx q[10], q[7];
h q[11];
h q[0];
t q[6];
t q[10];
t q[0];
cx q[10], q[6];
cx q[0], q[10];
cx q[6], q[0];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[0];
cx q[0], q[10];
cx q[6], q[0];
cx q[10], q[6];
h q[0];
h q[10];
t q[8];
t q[7];
t q[10];
cx q[7], q[8];
cx q[10], q[7];
cx q[8], q[10];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[10];
cx q[10], q[7];
cx q[8], q[10];
cx q[7], q[8];
h q[10];
h q[7];
t q[9];
t q[5];
t q[7];
cx q[5], q[9];
cx q[7], q[5];
cx q[9], q[7];
tdg q[5];
cx q[9], q[5];
tdg q[9];
tdg q[5];
t q[7];
cx q[7], q[5];
cx q[9], q[7];
cx q[5], q[9];
h q[7];
h q[5];
t q[12];
t q[11];
t q[5];
cx q[11], q[12];
cx q[5], q[11];
cx q[12], q[5];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[5];
cx q[5], q[11];
cx q[12], q[5];
cx q[11], q[12];
h q[5];
h q[7];
t q[9];
t q[5];
t q[7];
cx q[5], q[9];
cx q[7], q[5];
cx q[9], q[7];
tdg q[5];
cx q[9], q[5];
tdg q[9];
tdg q[5];
t q[7];
cx q[7], q[5];
cx q[9], q[7];
cx q[5], q[9];
h q[7];
h q[10];
t q[8];
t q[7];
t q[10];
cx q[7], q[8];
cx q[10], q[7];
cx q[8], q[10];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[10];
cx q[10], q[7];
cx q[8], q[10];
cx q[7], q[8];
h q[10];
h q[0];
t q[6];
t q[10];
t q[0];
cx q[10], q[6];
cx q[0], q[10];
cx q[6], q[0];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[0];
cx q[0], q[10];
cx q[6], q[0];
cx q[10], q[6];
h q[0];
h q[10];
t q[8];
t q[7];
t q[10];
cx q[7], q[8];
cx q[10], q[7];
cx q[8], q[10];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[10];
cx q[10], q[7];
cx q[8], q[10];
cx q[7], q[8];
h q[10];
h q[7];
t q[9];
t q[5];
t q[7];
cx q[5], q[9];
cx q[7], q[5];
cx q[9], q[7];
tdg q[5];
cx q[9], q[5];
tdg q[9];
tdg q[5];
t q[7];
cx q[7], q[5];
cx q[9], q[7];
cx q[5], q[9];
h q[7];
h q[5];
t q[12];
t q[11];
t q[5];
cx q[11], q[12];
cx q[5], q[11];
cx q[12], q[5];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[5];
cx q[5], q[11];
cx q[12], q[5];
cx q[11], q[12];
h q[5];
h q[7];
t q[9];
t q[5];
t q[7];
cx q[5], q[9];
cx q[7], q[5];
cx q[9], q[7];
tdg q[5];
cx q[9], q[5];
tdg q[9];
tdg q[5];
t q[7];
cx q[7], q[5];
cx q[9], q[7];
cx q[5], q[9];
h q[7];
h q[10];
t q[8];
t q[7];
t q[10];
cx q[7], q[8];
cx q[10], q[7];
cx q[8], q[10];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[10];
cx q[10], q[7];
cx q[8], q[10];
cx q[7], q[8];
h q[10];
h q[0];
t q[6];
t q[11];
t q[0];
cx q[11], q[6];
cx q[0], q[11];
cx q[6], q[0];
tdg q[11];
cx q[6], q[11];
tdg q[6];
tdg q[11];
t q[0];
cx q[0], q[11];
cx q[6], q[0];
cx q[11], q[6];
h q[0];
h q[11];
t q[7];
t q[10];
t q[11];
cx q[10], q[7];
cx q[11], q[10];
cx q[7], q[11];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[7], q[11];
cx q[10], q[7];
h q[11];
h q[10];
t q[8];
t q[5];
t q[10];
cx q[5], q[8];
cx q[10], q[5];
cx q[8], q[10];
tdg q[5];
cx q[8], q[5];
tdg q[8];
tdg q[5];
t q[10];
cx q[10], q[5];
cx q[8], q[10];
cx q[5], q[8];
h q[10];
h q[5];
t q[12];
t q[9];
t q[5];
cx q[9], q[12];
cx q[5], q[9];
cx q[12], q[5];
tdg q[9];
cx q[12], q[9];
tdg q[12];
tdg q[9];
t q[5];
cx q[5], q[9];
cx q[12], q[5];
cx q[9], q[12];
h q[5];
h q[10];
t q[8];
t q[5];
t q[10];
cx q[5], q[8];
cx q[10], q[5];
cx q[8], q[10];
tdg q[5];
cx q[8], q[5];
tdg q[8];
tdg q[5];
t q[10];
cx q[10], q[5];
cx q[8], q[10];
cx q[5], q[8];
h q[10];
h q[11];
t q[7];
t q[10];
t q[11];
cx q[10], q[7];
cx q[11], q[10];
cx q[7], q[11];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[7], q[11];
cx q[10], q[7];
h q[11];
h q[0];
t q[6];
t q[11];
t q[0];
cx q[11], q[6];
cx q[0], q[11];
cx q[6], q[0];
tdg q[11];
cx q[6], q[11];
tdg q[6];
tdg q[11];
t q[0];
cx q[0], q[11];
cx q[6], q[0];
cx q[11], q[6];
h q[0];
h q[11];
t q[7];
t q[10];
t q[11];
cx q[10], q[7];
cx q[11], q[10];
cx q[7], q[11];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[7], q[11];
cx q[10], q[7];
h q[11];
h q[10];
t q[8];
t q[5];
t q[10];
cx q[5], q[8];
cx q[10], q[5];
cx q[8], q[10];
tdg q[5];
cx q[8], q[5];
tdg q[8];
tdg q[5];
t q[10];
cx q[10], q[5];
cx q[8], q[10];
cx q[5], q[8];
h q[10];
h q[5];
t q[12];
t q[9];
t q[5];
cx q[9], q[12];
cx q[5], q[9];
cx q[12], q[5];
tdg q[9];
cx q[12], q[9];
tdg q[12];
tdg q[9];
t q[5];
cx q[5], q[9];
cx q[12], q[5];
cx q[9], q[12];
h q[5];
h q[10];
t q[8];
t q[5];
t q[10];
cx q[5], q[8];
cx q[10], q[5];
cx q[8], q[10];
tdg q[5];
cx q[8], q[5];
tdg q[8];
tdg q[5];
t q[10];
cx q[10], q[5];
cx q[8], q[10];
cx q[5], q[8];
h q[10];
h q[11];
t q[7];
t q[10];
t q[11];
cx q[10], q[7];
cx q[11], q[10];
cx q[7], q[11];
tdg q[10];
cx q[7], q[10];
tdg q[7];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[7], q[11];
cx q[10], q[7];
h q[11];
x q[10];
h q[0];
t q[7];
t q[11];
t q[0];
cx q[11], q[7];
cx q[0], q[11];
cx q[7], q[0];
tdg q[11];
cx q[7], q[11];
tdg q[7];
tdg q[11];
t q[0];
cx q[0], q[11];
cx q[7], q[0];
cx q[11], q[7];
h q[0];
h q[11];
t q[8];
t q[6];
t q[11];
cx q[6], q[8];
cx q[11], q[6];
cx q[8], q[11];
tdg q[6];
cx q[8], q[6];
tdg q[8];
tdg q[6];
t q[11];
cx q[11], q[6];
cx q[8], q[11];
cx q[6], q[8];
h q[11];
h q[6];
t q[9];
t q[5];
t q[6];
cx q[5], q[9];
cx q[6], q[5];
cx q[9], q[6];
tdg q[5];
cx q[9], q[5];
tdg q[9];
tdg q[5];
t q[6];
cx q[6], q[5];
cx q[9], q[6];
cx q[5], q[9];
h q[6];
h q[5];
t q[12];
t q[10];
t q[5];
cx q[10], q[12];
cx q[5], q[10];
cx q[12], q[5];
tdg q[10];
cx q[12], q[10];
tdg q[12];
tdg q[10];
t q[5];
cx q[5], q[10];
cx q[12], q[5];
cx q[10], q[12];
h q[5];
h q[6];
t q[9];
t q[5];
t q[6];
cx q[5], q[9];
cx q[6], q[5];
cx q[9], q[6];
tdg q[5];
cx q[9], q[5];
tdg q[9];
tdg q[5];
t q[6];
cx q[6], q[5];
cx q[9], q[6];
cx q[5], q[9];
h q[6];
h q[11];
t q[8];
t q[6];
t q[11];
cx q[6], q[8];
cx q[11], q[6];
cx q[8], q[11];
tdg q[6];
cx q[8], q[6];
tdg q[8];
tdg q[6];
t q[11];
cx q[11], q[6];
cx q[8], q[11];
cx q[6], q[8];
h q[11];
h q[0];
t q[7];
t q[11];
t q[0];
cx q[11], q[7];
cx q[0], q[11];
cx q[7], q[0];
tdg q[11];
cx q[7], q[11];
tdg q[7];
tdg q[11];
t q[0];
cx q[0], q[11];
cx q[7], q[0];
cx q[11], q[7];
h q[0];
h q[11];
t q[8];
t q[6];
t q[11];
cx q[6], q[8];
cx q[11], q[6];
cx q[8], q[11];
tdg q[6];
cx q[8], q[6];
tdg q[8];
tdg q[6];
t q[11];
cx q[11], q[6];
cx q[8], q[11];
cx q[6], q[8];
h q[11];
h q[6];
t q[9];
t q[5];
t q[6];
cx q[5], q[9];
cx q[6], q[5];
cx q[9], q[6];
tdg q[5];
cx q[9], q[5];
tdg q[9];
tdg q[5];
t q[6];
cx q[6], q[5];
cx q[9], q[6];
cx q[5], q[9];
h q[6];
h q[5];
t q[12];
t q[10];
t q[5];
cx q[10], q[12];
cx q[5], q[10];
cx q[12], q[5];
tdg q[10];
cx q[12], q[10];
tdg q[12];
tdg q[10];
t q[5];
cx q[5], q[10];
cx q[12], q[5];
cx q[10], q[12];
h q[5];
h q[6];
t q[9];
t q[5];
t q[6];
cx q[5], q[9];
cx q[6], q[5];
cx q[9], q[6];
tdg q[5];
cx q[9], q[5];
tdg q[9];
tdg q[5];
t q[6];
cx q[6], q[5];
cx q[9], q[6];
cx q[5], q[9];
h q[6];
h q[11];
t q[8];
t q[6];
t q[11];
cx q[6], q[8];
cx q[11], q[6];
cx q[8], q[11];
tdg q[6];
cx q[8], q[6];
tdg q[8];
tdg q[6];
t q[11];
cx q[11], q[6];
cx q[8], q[11];
cx q[6], q[8];
h q[11];
h q[0];
t q[8];
t q[7];
t q[0];
cx q[7], q[8];
cx q[0], q[7];
cx q[8], q[0];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[0];
cx q[0], q[7];
cx q[8], q[0];
cx q[7], q[8];
h q[0];
h q[7];
t q[9];
t q[6];
t q[7];
cx q[6], q[9];
cx q[7], q[6];
cx q[9], q[7];
tdg q[6];
cx q[9], q[6];
tdg q[9];
tdg q[6];
t q[7];
cx q[7], q[6];
cx q[9], q[7];
cx q[6], q[9];
h q[7];
h q[6];
t q[10];
t q[5];
t q[6];
cx q[5], q[10];
cx q[6], q[5];
cx q[10], q[6];
tdg q[5];
cx q[10], q[5];
tdg q[10];
tdg q[5];
t q[6];
cx q[6], q[5];
cx q[10], q[6];
cx q[5], q[10];
h q[6];
h q[5];
t q[12];
t q[11];
t q[5];
cx q[11], q[12];
cx q[5], q[11];
cx q[12], q[5];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[5];
cx q[5], q[11];
cx q[12], q[5];
cx q[11], q[12];
h q[5];
h q[6];
t q[10];
t q[5];
t q[6];
cx q[5], q[10];
cx q[6], q[5];
cx q[10], q[6];
tdg q[5];
cx q[10], q[5];
tdg q[10];
tdg q[5];
t q[6];
cx q[6], q[5];
cx q[10], q[6];
cx q[5], q[10];
h q[6];
h q[7];
t q[9];
t q[6];
t q[7];
cx q[6], q[9];
cx q[7], q[6];
cx q[9], q[7];
tdg q[6];
cx q[9], q[6];
tdg q[9];
tdg q[6];
t q[7];
cx q[7], q[6];
cx q[9], q[7];
cx q[6], q[9];
h q[7];
h q[0];
t q[8];
t q[7];
t q[0];
cx q[7], q[8];
cx q[0], q[7];
cx q[8], q[0];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[0];
cx q[0], q[7];
cx q[8], q[0];
cx q[7], q[8];
h q[0];
h q[7];
t q[9];
t q[6];
t q[7];
cx q[6], q[9];
cx q[7], q[6];
cx q[9], q[7];
tdg q[6];
cx q[9], q[6];
tdg q[9];
tdg q[6];
t q[7];
cx q[7], q[6];
cx q[9], q[7];
cx q[6], q[9];
h q[7];
h q[6];
t q[10];
t q[5];
t q[6];
cx q[5], q[10];
cx q[6], q[5];
cx q[10], q[6];
tdg q[5];
cx q[10], q[5];
tdg q[10];
tdg q[5];
t q[6];
cx q[6], q[5];
cx q[10], q[6];
cx q[5], q[10];
h q[6];
h q[5];
t q[12];
t q[11];
t q[5];
cx q[11], q[12];
cx q[5], q[11];
cx q[12], q[5];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[5];
cx q[5], q[11];
cx q[12], q[5];
cx q[11], q[12];
h q[5];
h q[6];
t q[10];
t q[5];
t q[6];
cx q[5], q[10];
cx q[6], q[5];
cx q[10], q[6];
tdg q[5];
cx q[10], q[5];
tdg q[10];
tdg q[5];
t q[6];
cx q[6], q[5];
cx q[10], q[6];
cx q[5], q[10];
h q[6];
h q[7];
t q[9];
t q[6];
t q[7];
cx q[6], q[9];
cx q[7], q[6];
cx q[9], q[7];
tdg q[6];
cx q[9], q[6];
tdg q[9];
tdg q[6];
t q[7];
cx q[7], q[6];
cx q[9], q[7];
cx q[6], q[9];
h q[7];
h q[2];
t q[10];
t q[6];
t q[2];
cx q[6], q[10];
cx q[2], q[6];
cx q[10], q[2];
tdg q[6];
cx q[10], q[6];
tdg q[10];
tdg q[6];
t q[2];
cx q[2], q[6];
cx q[10], q[2];
cx q[6], q[10];
h q[2];
x q[7];
h q[2];
t q[7];
t q[11];
t q[2];
cx q[11], q[7];
cx q[2], q[11];
cx q[7], q[2];
tdg q[11];
cx q[7], q[11];
tdg q[7];
tdg q[11];
t q[2];
cx q[2], q[11];
cx q[7], q[2];
cx q[11], q[7];
h q[2];
h q[11];
t q[12];
t q[8];
t q[11];
cx q[8], q[12];
cx q[11], q[8];
cx q[12], q[11];
tdg q[8];
cx q[12], q[8];
tdg q[12];
tdg q[8];
t q[11];
cx q[11], q[8];
cx q[12], q[11];
cx q[8], q[12];
h q[11];
h q[2];
t q[7];
t q[11];
t q[2];
cx q[11], q[7];
cx q[2], q[11];
cx q[7], q[2];
tdg q[11];
cx q[7], q[11];
tdg q[7];
tdg q[11];
t q[2];
cx q[2], q[11];
cx q[7], q[2];
cx q[11], q[7];
h q[2];
h q[11];
t q[12];
t q[8];
t q[11];
cx q[8], q[12];
cx q[11], q[8];
cx q[12], q[11];
tdg q[8];
cx q[12], q[8];
tdg q[12];
tdg q[8];
t q[11];
cx q[11], q[8];
cx q[12], q[11];
cx q[8], q[12];
h q[11];
h q[1];
t q[7];
t q[11];
t q[1];
cx q[11], q[7];
cx q[1], q[11];
cx q[7], q[1];
tdg q[11];
cx q[7], q[11];
tdg q[7];
tdg q[11];
t q[1];
cx q[1], q[11];
cx q[7], q[1];
cx q[11], q[7];
h q[1];
h q[11];
t q[8];
t q[9];
t q[11];
cx q[9], q[8];
cx q[11], q[9];
cx q[8], q[11];
tdg q[9];
cx q[8], q[9];
tdg q[8];
tdg q[9];
t q[11];
cx q[11], q[9];
cx q[8], q[11];
cx q[9], q[8];
h q[11];
h q[9];
t q[12];
t q[10];
t q[9];
cx q[10], q[12];
cx q[9], q[10];
cx q[12], q[9];
tdg q[10];
cx q[12], q[10];
tdg q[12];
tdg q[10];
t q[9];
cx q[9], q[10];
cx q[12], q[9];
cx q[10], q[12];
h q[9];
h q[11];
t q[8];
t q[9];
t q[11];
cx q[9], q[8];
cx q[11], q[9];
cx q[8], q[11];
tdg q[9];
cx q[8], q[9];
tdg q[8];
tdg q[9];
t q[11];
cx q[11], q[9];
cx q[8], q[11];
cx q[9], q[8];
h q[11];
h q[1];
t q[7];
t q[11];
t q[1];
cx q[11], q[7];
cx q[1], q[11];
cx q[7], q[1];
tdg q[11];
cx q[7], q[11];
tdg q[7];
tdg q[11];
t q[1];
cx q[1], q[11];
cx q[7], q[1];
cx q[11], q[7];
h q[1];
h q[11];
t q[8];
t q[9];
t q[11];
cx q[9], q[8];
cx q[11], q[9];
cx q[8], q[11];
tdg q[9];
cx q[8], q[9];
tdg q[8];
tdg q[9];
t q[11];
cx q[11], q[9];
cx q[8], q[11];
cx q[9], q[8];
h q[11];
h q[9];
t q[12];
t q[10];
t q[9];
cx q[10], q[12];
cx q[9], q[10];
cx q[12], q[9];
tdg q[10];
cx q[12], q[10];
tdg q[12];
tdg q[10];
t q[9];
cx q[9], q[10];
cx q[12], q[9];
cx q[10], q[12];
h q[9];
h q[11];
t q[8];
t q[9];
t q[11];
cx q[9], q[8];
cx q[11], q[9];
cx q[8], q[11];
tdg q[9];
cx q[8], q[9];
tdg q[8];
tdg q[9];
t q[11];
cx q[11], q[9];
cx q[8], q[11];
cx q[9], q[8];
h q[11];
h q[0];
t q[5];
t q[11];
t q[0];
cx q[11], q[5];
cx q[0], q[11];
cx q[5], q[0];
tdg q[11];
cx q[5], q[11];
tdg q[5];
tdg q[11];
t q[0];
cx q[0], q[11];
cx q[5], q[0];
cx q[11], q[5];
h q[0];
h q[11];
t q[6];
t q[10];
t q[11];
cx q[10], q[6];
cx q[11], q[10];
cx q[6], q[11];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[6], q[11];
cx q[10], q[6];
h q[11];
h q[10];
t q[7];
t q[9];
t q[10];
cx q[9], q[7];
cx q[10], q[9];
cx q[7], q[10];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[7], q[10];
cx q[9], q[7];
h q[10];
h q[9];
t q[12];
t q[8];
t q[9];
cx q[8], q[12];
cx q[9], q[8];
cx q[12], q[9];
tdg q[8];
cx q[12], q[8];
tdg q[12];
tdg q[8];
t q[9];
cx q[9], q[8];
cx q[12], q[9];
cx q[8], q[12];
h q[9];
h q[10];
t q[7];
t q[9];
t q[10];
cx q[9], q[7];
cx q[10], q[9];
cx q[7], q[10];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[7], q[10];
cx q[9], q[7];
h q[10];
h q[11];
t q[6];
t q[10];
t q[11];
cx q[10], q[6];
cx q[11], q[10];
cx q[6], q[11];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[6], q[11];
cx q[10], q[6];
h q[11];
h q[0];
t q[5];
t q[11];
t q[0];
cx q[11], q[5];
cx q[0], q[11];
cx q[5], q[0];
tdg q[11];
cx q[5], q[11];
tdg q[5];
tdg q[11];
t q[0];
cx q[0], q[11];
cx q[5], q[0];
cx q[11], q[5];
h q[0];
h q[11];
t q[6];
t q[10];
t q[11];
cx q[10], q[6];
cx q[11], q[10];
cx q[6], q[11];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[6], q[11];
cx q[10], q[6];
h q[11];
h q[10];
t q[7];
t q[9];
t q[10];
cx q[9], q[7];
cx q[10], q[9];
cx q[7], q[10];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[7], q[10];
cx q[9], q[7];
h q[10];
h q[9];
t q[12];
t q[8];
t q[9];
cx q[8], q[12];
cx q[9], q[8];
cx q[12], q[9];
tdg q[8];
cx q[12], q[8];
tdg q[12];
tdg q[8];
t q[9];
cx q[9], q[8];
cx q[12], q[9];
cx q[8], q[12];
h q[9];
h q[10];
t q[7];
t q[9];
t q[10];
cx q[9], q[7];
cx q[10], q[9];
cx q[7], q[10];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[7], q[10];
cx q[9], q[7];
h q[10];
h q[11];
t q[6];
t q[10];
t q[11];
cx q[10], q[6];
cx q[11], q[10];
cx q[6], q[11];
tdg q[10];
cx q[6], q[10];
tdg q[6];
tdg q[10];
t q[11];
cx q[11], q[10];
cx q[6], q[11];
cx q[10], q[6];
h q[11];
h q[0];
t q[5];
t q[11];
t q[0];
cx q[11], q[5];
cx q[0], q[11];
cx q[5], q[0];
tdg q[11];
cx q[5], q[11];
tdg q[5];
tdg q[11];
t q[0];
cx q[0], q[11];
cx q[5], q[0];
cx q[11], q[5];
h q[0];
h q[11];
t q[7];
t q[9];
t q[11];
cx q[9], q[7];
cx q[11], q[9];
cx q[7], q[11];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[11];
cx q[11], q[9];
cx q[7], q[11];
cx q[9], q[7];
h q[11];
h q[9];
t q[8];
t q[6];
t q[9];
cx q[6], q[8];
cx q[9], q[6];
cx q[8], q[9];
tdg q[6];
cx q[8], q[6];
tdg q[8];
tdg q[6];
t q[9];
cx q[9], q[6];
cx q[8], q[9];
cx q[6], q[8];
h q[9];
h q[6];
t q[12];
t q[10];
t q[6];
cx q[10], q[12];
cx q[6], q[10];
cx q[12], q[6];
tdg q[10];
cx q[12], q[10];
tdg q[12];
tdg q[10];
t q[6];
cx q[6], q[10];
cx q[12], q[6];
cx q[10], q[12];
h q[6];
h q[9];
t q[8];
t q[6];
t q[9];
cx q[6], q[8];
cx q[9], q[6];
cx q[8], q[9];
tdg q[6];
cx q[8], q[6];
tdg q[8];
tdg q[6];
t q[9];
cx q[9], q[6];
cx q[8], q[9];
cx q[6], q[8];
h q[9];
h q[11];
t q[7];
t q[9];
t q[11];
cx q[9], q[7];
cx q[11], q[9];
cx q[7], q[11];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[11];
cx q[11], q[9];
cx q[7], q[11];
cx q[9], q[7];
h q[11];
h q[0];
t q[5];
t q[11];
t q[0];
cx q[11], q[5];
cx q[0], q[11];
cx q[5], q[0];
tdg q[11];
cx q[5], q[11];
tdg q[5];
tdg q[11];
t q[0];
cx q[0], q[11];
cx q[5], q[0];
cx q[11], q[5];
h q[0];
h q[11];
t q[7];
t q[9];
t q[11];
cx q[9], q[7];
cx q[11], q[9];
cx q[7], q[11];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[11];
cx q[11], q[9];
cx q[7], q[11];
cx q[9], q[7];
h q[11];
h q[9];
t q[8];
t q[6];
t q[9];
cx q[6], q[8];
cx q[9], q[6];
cx q[8], q[9];
tdg q[6];
cx q[8], q[6];
tdg q[8];
tdg q[6];
t q[9];
cx q[9], q[6];
cx q[8], q[9];
cx q[6], q[8];
h q[9];
h q[6];
t q[12];
t q[10];
t q[6];
cx q[10], q[12];
cx q[6], q[10];
cx q[12], q[6];
tdg q[10];
cx q[12], q[10];
tdg q[12];
tdg q[10];
t q[6];
cx q[6], q[10];
cx q[12], q[6];
cx q[10], q[12];
h q[6];
h q[9];
t q[8];
t q[6];
t q[9];
cx q[6], q[8];
cx q[9], q[6];
cx q[8], q[9];
tdg q[6];
cx q[8], q[6];
tdg q[8];
tdg q[6];
t q[9];
cx q[9], q[6];
cx q[8], q[9];
cx q[6], q[8];
h q[9];
h q[11];
t q[7];
t q[9];
t q[11];
cx q[9], q[7];
cx q[11], q[9];
cx q[7], q[11];
tdg q[9];
cx q[7], q[9];
tdg q[7];
tdg q[9];
t q[11];
cx q[11], q[9];
cx q[7], q[11];
cx q[9], q[7];
h q[11];
x q[11];
h q[3];
t q[11];
t q[7];
t q[3];
cx q[7], q[11];
cx q[3], q[7];
cx q[11], q[3];
tdg q[7];
cx q[11], q[7];
tdg q[11];
tdg q[7];
t q[3];
cx q[3], q[7];
cx q[11], q[3];
cx q[7], q[11];
h q[3];
h q[1];
t q[8];
t q[9];
t q[1];
cx q[9], q[8];
cx q[1], q[9];
cx q[8], q[1];
tdg q[9];
cx q[8], q[9];
tdg q[8];
tdg q[9];
t q[1];
cx q[1], q[9];
cx q[8], q[1];
cx q[9], q[8];
h q[1];
h q[9];
t q[10];
t q[7];
t q[9];
cx q[7], q[10];
cx q[9], q[7];
cx q[10], q[9];
tdg q[7];
cx q[10], q[7];
tdg q[10];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[10], q[9];
cx q[7], q[10];
h q[9];
h q[7];
t q[12];
t q[11];
t q[7];
cx q[11], q[12];
cx q[7], q[11];
cx q[12], q[7];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[7];
cx q[7], q[11];
cx q[12], q[7];
cx q[11], q[12];
h q[7];
h q[9];
t q[10];
t q[7];
t q[9];
cx q[7], q[10];
cx q[9], q[7];
cx q[10], q[9];
tdg q[7];
cx q[10], q[7];
tdg q[10];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[10], q[9];
cx q[7], q[10];
h q[9];
h q[1];
t q[8];
t q[9];
t q[1];
cx q[9], q[8];
cx q[1], q[9];
cx q[8], q[1];
tdg q[9];
cx q[8], q[9];
tdg q[8];
tdg q[9];
t q[1];
cx q[1], q[9];
cx q[8], q[1];
cx q[9], q[8];
h q[1];
h q[9];
t q[10];
t q[7];
t q[9];
cx q[7], q[10];
cx q[9], q[7];
cx q[10], q[9];
tdg q[7];
cx q[10], q[7];
tdg q[10];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[10], q[9];
cx q[7], q[10];
h q[9];
h q[7];
t q[12];
t q[11];
t q[7];
cx q[11], q[12];
cx q[7], q[11];
cx q[12], q[7];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[7];
cx q[7], q[11];
cx q[12], q[7];
cx q[11], q[12];
h q[7];
h q[9];
t q[10];
t q[7];
t q[9];
cx q[7], q[10];
cx q[9], q[7];
cx q[10], q[9];
tdg q[7];
cx q[10], q[7];
tdg q[10];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[10], q[9];
cx q[7], q[10];
h q[9];
h q[1];
t q[8];
t q[10];
t q[1];
cx q[10], q[8];
cx q[1], q[10];
cx q[8], q[1];
tdg q[10];
cx q[8], q[10];
tdg q[8];
tdg q[10];
t q[1];
cx q[1], q[10];
cx q[8], q[1];
cx q[10], q[8];
h q[1];
h q[10];
t q[12];
t q[11];
t q[10];
cx q[11], q[12];
cx q[10], q[11];
cx q[12], q[10];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[10];
cx q[10], q[11];
cx q[12], q[10];
cx q[11], q[12];
h q[10];
h q[1];
t q[8];
t q[10];
t q[1];
cx q[10], q[8];
cx q[1], q[10];
cx q[8], q[1];
tdg q[10];
cx q[8], q[10];
tdg q[8];
tdg q[10];
t q[1];
cx q[1], q[10];
cx q[8], q[1];
cx q[10], q[8];
h q[1];
h q[10];
t q[12];
t q[11];
t q[10];
cx q[11], q[12];
cx q[10], q[11];
cx q[12], q[10];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[10];
cx q[10], q[11];
cx q[12], q[10];
cx q[11], q[12];
h q[10];
h q[0];
t q[5];
t q[10];
t q[0];
cx q[10], q[5];
cx q[0], q[10];
cx q[5], q[0];
tdg q[10];
cx q[5], q[10];
tdg q[5];
tdg q[10];
t q[0];
cx q[0], q[10];
cx q[5], q[0];
cx q[10], q[5];
h q[0];
h q[10];
t q[6];
t q[9];
t q[10];
cx q[9], q[6];
cx q[10], q[9];
cx q[6], q[10];
tdg q[9];
cx q[6], q[9];
tdg q[6];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[6], q[10];
cx q[9], q[6];
h q[10];
h q[9];
t q[8];
t q[7];
t q[9];
cx q[7], q[8];
cx q[9], q[7];
cx q[8], q[9];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[8], q[9];
cx q[7], q[8];
h q[9];
h q[7];
t q[12];
t q[11];
t q[7];
cx q[11], q[12];
cx q[7], q[11];
cx q[12], q[7];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[7];
cx q[7], q[11];
cx q[12], q[7];
cx q[11], q[12];
h q[7];
h q[9];
t q[8];
t q[7];
t q[9];
cx q[7], q[8];
cx q[9], q[7];
cx q[8], q[9];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[8], q[9];
cx q[7], q[8];
h q[9];
h q[10];
t q[6];
t q[9];
t q[10];
cx q[9], q[6];
cx q[10], q[9];
cx q[6], q[10];
tdg q[9];
cx q[6], q[9];
tdg q[6];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[6], q[10];
cx q[9], q[6];
h q[10];
h q[0];
t q[5];
t q[10];
t q[0];
cx q[10], q[5];
cx q[0], q[10];
cx q[5], q[0];
tdg q[10];
cx q[5], q[10];
tdg q[5];
tdg q[10];
t q[0];
cx q[0], q[10];
cx q[5], q[0];
cx q[10], q[5];
h q[0];
h q[10];
t q[6];
t q[9];
t q[10];
cx q[9], q[6];
cx q[10], q[9];
cx q[6], q[10];
tdg q[9];
cx q[6], q[9];
tdg q[6];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[6], q[10];
cx q[9], q[6];
h q[10];
h q[9];
t q[8];
t q[7];
t q[9];
cx q[7], q[8];
cx q[9], q[7];
cx q[8], q[9];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[8], q[9];
cx q[7], q[8];
h q[9];
h q[7];
t q[12];
t q[11];
t q[7];
cx q[11], q[12];
cx q[7], q[11];
cx q[12], q[7];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[7];
cx q[7], q[11];
cx q[12], q[7];
cx q[11], q[12];
h q[7];
h q[9];
t q[8];
t q[7];
t q[9];
cx q[7], q[8];
cx q[9], q[7];
cx q[8], q[9];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[8], q[9];
cx q[7], q[8];
h q[9];
h q[10];
t q[6];
t q[9];
t q[10];
cx q[9], q[6];
cx q[10], q[9];
cx q[6], q[10];
tdg q[9];
cx q[6], q[9];
tdg q[6];
tdg q[9];
t q[10];
cx q[10], q[9];
cx q[6], q[10];
cx q[9], q[6];
h q[10];
h q[0];
t q[5];
t q[9];
t q[0];
cx q[9], q[5];
cx q[0], q[9];
cx q[5], q[0];
tdg q[9];
cx q[5], q[9];
tdg q[5];
tdg q[9];
t q[0];
cx q[0], q[9];
cx q[5], q[0];
cx q[9], q[5];
h q[0];
h q[9];
t q[8];
t q[7];
t q[9];
cx q[7], q[8];
cx q[9], q[7];
cx q[8], q[9];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[8], q[9];
cx q[7], q[8];
h q[9];
h q[7];
t q[10];
t q[6];
t q[7];
cx q[6], q[10];
cx q[7], q[6];
cx q[10], q[7];
tdg q[6];
cx q[10], q[6];
tdg q[10];
tdg q[6];
t q[7];
cx q[7], q[6];
cx q[10], q[7];
cx q[6], q[10];
h q[7];
h q[6];
t q[12];
t q[11];
t q[6];
cx q[11], q[12];
cx q[6], q[11];
cx q[12], q[6];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[6];
cx q[6], q[11];
cx q[12], q[6];
cx q[11], q[12];
h q[6];
h q[7];
t q[10];
t q[6];
t q[7];
cx q[6], q[10];
cx q[7], q[6];
cx q[10], q[7];
tdg q[6];
cx q[10], q[6];
tdg q[10];
tdg q[6];
t q[7];
cx q[7], q[6];
cx q[10], q[7];
cx q[6], q[10];
h q[7];
h q[9];
t q[8];
t q[7];
t q[9];
cx q[7], q[8];
cx q[9], q[7];
cx q[8], q[9];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[8], q[9];
cx q[7], q[8];
h q[9];
h q[0];
t q[5];
t q[9];
t q[0];
cx q[9], q[5];
cx q[0], q[9];
cx q[5], q[0];
tdg q[9];
cx q[5], q[9];
tdg q[5];
tdg q[9];
t q[0];
cx q[0], q[9];
cx q[5], q[0];
cx q[9], q[5];
h q[0];
h q[9];
t q[8];
t q[7];
t q[9];
cx q[7], q[8];
cx q[9], q[7];
cx q[8], q[9];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[8], q[9];
cx q[7], q[8];
h q[9];
h q[7];
t q[10];
t q[6];
t q[7];
cx q[6], q[10];
cx q[7], q[6];
cx q[10], q[7];
tdg q[6];
cx q[10], q[6];
tdg q[10];
tdg q[6];
t q[7];
cx q[7], q[6];
cx q[10], q[7];
cx q[6], q[10];
h q[7];
h q[6];
t q[12];
t q[11];
t q[6];
cx q[11], q[12];
cx q[6], q[11];
cx q[12], q[6];
tdg q[11];
cx q[12], q[11];
tdg q[12];
tdg q[11];
t q[6];
cx q[6], q[11];
cx q[12], q[6];
cx q[11], q[12];
h q[6];
h q[7];
t q[10];
t q[6];
t q[7];
cx q[6], q[10];
cx q[7], q[6];
cx q[10], q[7];
tdg q[6];
cx q[10], q[6];
tdg q[10];
tdg q[6];
t q[7];
cx q[7], q[6];
cx q[10], q[7];
cx q[6], q[10];
h q[7];
h q[9];
t q[8];
t q[7];
t q[9];
cx q[7], q[8];
cx q[9], q[7];
cx q[8], q[9];
tdg q[7];
cx q[8], q[7];
tdg q[8];
tdg q[7];
t q[9];
cx q[9], q[7];
cx q[8], q[9];
cx q[7], q[8];
h q[9];
x q[8];
cx q[8], q[4];
h q[3];
t q[12];
t q[8];
t q[3];
cx q[8], q[12];
cx q[3], q[8];
cx q[12], q[3];
tdg q[8];
cx q[12], q[8];
tdg q[12];
tdg q[8];
t q[3];
cx q[3], q[8];
cx q[12], q[3];
cx q[8], q[12];
h q[3];
h q[2];
t q[12];
t q[8];
t q[2];
cx q[8], q[12];
cx q[2], q[8];
cx q[12], q[2];
tdg q[8];
cx q[12], q[8];
tdg q[12];
tdg q[8];
t q[2];
cx q[2], q[8];
cx q[12], q[2];
cx q[8], q[12];
h q[2];
h q[1];
t q[12];
t q[8];
t q[1];
cx q[8], q[12];
cx q[1], q[8];
cx q[12], q[1];
tdg q[8];
cx q[12], q[8];
tdg q[12];
tdg q[8];
t q[1];
cx q[1], q[8];
cx q[12], q[1];
cx q[8], q[12];
h q[1];
x q[12];
cx q[12], q[4];
cx q[12], q[3];
cx q[12], q[2];
cx q[12], q[1];
