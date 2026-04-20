# int A[30] = {...}#
# int B[30] = {...}#
# int C[30] = {...}#
# 
# for (int i = 0# i < 30# i++) {
#   C[i] = A[i] + B[i]#
# }

.A: 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9     # int A[30] = {...}
.B: 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9     # int B[30] = {...}
.C: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0     # int C[30] = {...}


    addi x31 x0 30           # int n = 30
    addi x1 x0 0            # int i = 1=0

for:
    lw x3 A(x1)             # x3 = A[i]
    lw x4 B(x1)             # x4 = B[i]
    add x5 x3 x4            # x5 = A[i] + B[i]
    sw x5 C(x1)             # c[i] = x5
    addi x1 x1 1            # i++
    blt x1 x31 for         # if (i < n) for loop again