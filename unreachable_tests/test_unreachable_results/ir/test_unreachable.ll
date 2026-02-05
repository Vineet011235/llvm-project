; ModuleID = 'test_unreachable.ll'
source_filename = "test_unreachable.ll"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @test_with_unreachable(i32 %x) {
entry:
  %cmp = icmp sgt i32 %x, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %mul = mul i32 %x, 2
  ret i32 %mul

if.else:                                          ; preds = %entry
  ret i32 0

unreachable_block:                                ; No predecessors!
  %add = add i32 %x, 100
  ret i32 %add
}

define i32 @another_test(i32 %a, i32 %b) {
start:
  %sum = add i32 %a, %b
  ret i32 %sum

dead_code:                                        ; No predecessors!
  %product = mul i32 %a, %b
  ret i32 %product

more_dead_code:                                   ; No predecessors!
  %diff = sub i32 %a, %b
  ret i32 %diff
}
