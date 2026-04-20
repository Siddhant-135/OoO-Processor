.A: 8 14 7 22 15 1 25 13 23 24 2 25 30 9 19 28 3 23 21 19 28 24 9 6 29 12 4 29 19 24

    addi x10 x10 30         
    addi x1 x1 1            

while:
    blt x10 x1 while_done
    add x2 x0 x0            
    add x3 x1 x0            
for:
    sub x4 x3 x1            
    lw x5 A(x4)             
    lw x6 A(x3)             
    ble x5 x6 less_than  
    addi x7 x5 0            
    sw x6 A(x4)            
    sw x7 A(x3)         
    addi x2 x3 0
less_than:
    addi x3 x3 1
    blt x3 x10 for
for_done:
    addi x10 x2 0
    j while

while_done:
    addi x31 x31 1