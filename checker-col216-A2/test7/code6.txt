# int r[280 + 1];
# int i, k;
# int b, d;
# int c = 0;

# for (i = 0# i < 280# i++) {
#   r[i] = 2000;
#   }

#   for (k = 280# k > 0# k -= 14) {
#   d = 0;
#   i = k;
#   while (1) {
#     d += r[i] * 10000;
#     b = 2 * i - 1;

#     r[i] = d % b;
#     d /= b;
#     i--;
#     if (i == 0) break;
#     d *= i;
#   }
#   printf("%d ", c + d / 10000);
#   c = d % 10000;
# }

# x0 = 0
# x1 = 1
# x2 = 2
# x3 = i
# x4 = k
# x5 = b
# x6 = d
# x15 = c
# x10 = j

.r: 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000 2000
.o: 0 0 0 0

    addi x1 x0 1       # x1 = 1
    addi x2 x0 2       # x2 = 2
    addi x4 x0 56      # k = 56
    addi x8 x0 10000   # x8 = 10000

for:
    addi x6 x0 0       # d = 0
    addi x3 x4 0       # i = k
while:
    lw x7 r(x3)        # x7 = r[i]
    mul x5 x2 x3      # b = 2 * i
    sub x5 x5 x1       # b = 2 * i - 1
    mul x9 x7 x8      # x9 = r[i] * 10000
    add x6 x6 x9       # d += r[i] * 10000
    rem x16 x6 x5      # x16 = d % b
    sw x16 r(x3)       # r[i] = d % b
    div x6 x6 x5      # d = d / b
    sub x3 x3 x1       # i--
    beq x3 x0 break  # if (i == 0) break
    mul x6 x6 x3      # d = d * i
    j while
break:
    div x11 x6 x8     # x11 = d / 10000
    add x11 x11 x15    # x11 = c + d / 10000
    sw x11 o(x10)      # o[j] = c + d / 10000
    add x10 x10 x1     # j++
    rem x15 x6 x8      # c = d % 10000
    addi x4 x4 -14      # k -= 14
    blt x0 x4 for    # if (k > 0) for loop again