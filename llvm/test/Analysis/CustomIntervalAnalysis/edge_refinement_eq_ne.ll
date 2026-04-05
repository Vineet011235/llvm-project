; RUN: opt -passes=interval-analysis %s | FileCheck %s

define i32 @eq_ne_refine(i32 %x) {
entry:
  %cmp = icmp eq i32 %x, 3
  br i1 %cmp, label %then, label %else

then:
  %t = add nsw i32 %x, 1
  br label %exit

else:
  %e = sub nsw i32 %x, 1
  br label %exit

exit:
  %r = phi i32 [ %t, %then ], [ %e, %else ]
  ret i32 %r
}

; CHECK: Basic Block: %then
; CHECK: %x       | {3}         | {3}
; CHECK: Basic Block: %else
; CHECK: %x       | [-INF, 2] U [4, INF] | [-INF, 2] U [4, INF]
