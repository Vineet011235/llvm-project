; RUN: opt -passes=interval-analysis %s | FileCheck %s

define i32 @loop_bounds() {
entry:
  br label %for.cond

for.cond:
  %a.0 = phi i32 [ 10, %entry ], [ %add, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, 5
  br i1 %cmp, label %for.body, label %for.end

for.body:
  %add = add nsw i32 %a.0, 1
  br label %for.inc

for.inc:
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:
  %mul = mul nsw i32 %a.0, 2
  ret i32 %mul
}

; CHECK: Basic Block: %for.cond
; CHECK: %i.0     | [0, 5]      | [0, 5]
; CHECK: Basic Block: %for.body
; CHECK: %i.0     | [0, 4]      | [0, 4]
; CHECK: Basic Block: %for.end
; CHECK: %i.0     | [5, INF]    | [5, INF]
