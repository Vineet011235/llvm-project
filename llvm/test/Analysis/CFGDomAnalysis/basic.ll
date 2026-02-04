; RUN: opt -passes=cfg-dom-analysis -disable-output < %s | FileCheck %s

; Simple test function with one conditional branch and one exit
define i32 @simple_function(i32 %x) {
; CHECK: Function: simple_function
; CHECK: BasicBlocks:
entry:
; CHECK-DAG:   entry
  %cmp = icmp sgt i32 %x, 0
  br i1 %cmp, label %then, label %exit

then:
; CHECK-DAG:   then
  %val = add i32 %x, 1
  br label %exit

exit:
; CHECK-DAG:   exit
  %result = phi i32 [ %val, %then ], [ %x, %entry ]
  ret i32 %result
}
