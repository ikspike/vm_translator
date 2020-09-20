// SP=256
@256
D=A
@SP
M=D
// call Sys.init
@RETADDR.SYS.INIT.0
D=A
@SP
AM=M+1
A=A-1
M=D
@LCL
D=M
@SP
AM=M+1
A=A-1
M=D
@ARG
D=M
@SP
AM=M+1
A=A-1
M=D
@THIS
D=M
@SP
AM=M+1
A=A-1
M=D
@THAT
D=M
@SP
AM=M+1
A=A-1
M=D
@SP
D=M
@5
D=D-A
@0
D=D-A
@ARG
M=D
@SP
D=M
@LCL
M=D
@SYS.INIT
0;JMP
(RETADDR.SYS.INIT.0)
// function Sys.init 0
(SYS.INIT)
// push constant 4
@4
D=A
@SP
AM=M+1
A=A-1
M=D
// call Main.fibonacci 1
@RETADDR.MAIN.FIBONACCI.1
D=A
@SP
AM=M+1
A=A-1
M=D
@LCL
D=M
@SP
AM=M+1
A=A-1
M=D
@ARG
D=M
@SP
AM=M+1
A=A-1
M=D
@THIS
D=M
@SP
AM=M+1
A=A-1
M=D
@THAT
D=M
@SP
AM=M+1
A=A-1
M=D
@SP
D=M
@5
D=D-A
@1
D=D-A
@ARG
M=D
@SP
D=M
@LCL
M=D
@MAIN.FIBONACCI
0;JMP
(RETADDR.MAIN.FIBONACCI.1)
// label WHILE
(SYS.WHILE)
// goto WHILE
@SYS.WHILE
0;JMP
// function Main.fibonacci 0
(MAIN.FIBONACCI)
// push argument 0
@ARG
D=M
@0
D=D+A
A=D
D=M
@SP
AM=M+1
A=A-1
M=D
// push constant 2
@2
D=A
@SP
AM=M+1
A=A-1
M=D
// lt
@SP
AM=M-1
D=M
A=A-1
D=M-D
@_TRUE0
D;JLT
D=0
@_FALSE0
0;JMP
(_TRUE0)
D=-1
(_FALSE0)
@SP
A=M-1
M=D
// if-goto IF_TRUE
@SP
AM=M-1
D=M
@MAIN.IF_TRUE
D;JNE
// goto IF_FALSE
@MAIN.IF_FALSE
0;JMP
// label IF_TRUE
(MAIN.IF_TRUE)
// push argument 0
@ARG
D=M
@0
D=D+A
A=D
D=M
@SP
AM=M+1
A=A-1
M=D
// return
@LCL
D=M
@endFrame
M=D
@5
A=D-A
D=M
@retAddr
M=D
@SP
A=M-1
D=M
@ARG
A=M
M=D
@ARG
D=M
@SP
M=D+1
@endFrame
D=M
@1
A=D-A
D=M
@THAT
M=D
@endFrame
D=M
@2
A=D-A
D=M
@THIS
M=D
@endFrame
D=M
@3
A=D-A
D=M
@ARG
M=D
@endFrame
D=M
@4
A=D-A
D=M
@LCL
M=D
@retAddr
A=M
0;JMP
// label IF_FALSE
(MAIN.IF_FALSE)
// push argument 0
@ARG
D=M
@0
D=D+A
A=D
D=M
@SP
AM=M+1
A=A-1
M=D
// push constant 2
@2
D=A
@SP
AM=M+1
A=A-1
M=D
// sub
@SP
AM=M-1
D=M
@SP
A=M-1
M=M-D
// call Main.fibonacci 1
@RETADDR.MAIN.FIBONACCI.2
D=A
@SP
AM=M+1
A=A-1
M=D
@LCL
D=M
@SP
AM=M+1
A=A-1
M=D
@ARG
D=M
@SP
AM=M+1
A=A-1
M=D
@THIS
D=M
@SP
AM=M+1
A=A-1
M=D
@THAT
D=M
@SP
AM=M+1
A=A-1
M=D
@SP
D=M
@5
D=D-A
@1
D=D-A
@ARG
M=D
@SP
D=M
@LCL
M=D
@MAIN.FIBONACCI
0;JMP
(RETADDR.MAIN.FIBONACCI.2)
// push argument 0
@ARG
D=M
@0
D=D+A
A=D
D=M
@SP
AM=M+1
A=A-1
M=D
// push constant 1
@1
D=A
@SP
AM=M+1
A=A-1
M=D
// sub
@SP
AM=M-1
D=M
@SP
A=M-1
M=M-D
// call Main.fibonacci 1
@RETADDR.MAIN.FIBONACCI.3
D=A
@SP
AM=M+1
A=A-1
M=D
@LCL
D=M
@SP
AM=M+1
A=A-1
M=D
@ARG
D=M
@SP
AM=M+1
A=A-1
M=D
@THIS
D=M
@SP
AM=M+1
A=A-1
M=D
@THAT
D=M
@SP
AM=M+1
A=A-1
M=D
@SP
D=M
@5
D=D-A
@1
D=D-A
@ARG
M=D
@SP
D=M
@LCL
M=D
@MAIN.FIBONACCI
0;JMP
(RETADDR.MAIN.FIBONACCI.3)
// add
@SP
AM=M-1
D=M
@SP
A=M-1
M=M+D
// return
@LCL
D=M
@endFrame
M=D
@5
A=D-A
D=M
@retAddr
M=D
@SP
A=M-1
D=M
@ARG
A=M
M=D
@ARG
D=M
@SP
M=D+1
@endFrame
D=M
@1
A=D-A
D=M
@THAT
M=D
@endFrame
D=M
@2
A=D-A
D=M
@THIS
M=D
@endFrame
D=M
@3
A=D-A
D=M
@ARG
M=D
@endFrame
D=M
@4
A=D-A
D=M
@LCL
M=D
@retAddr
A=M
0;JMP