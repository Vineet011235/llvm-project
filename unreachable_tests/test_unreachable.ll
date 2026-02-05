; Test function with deliberately unreachable basic blocks
define i32 @test_with_unreachable(i32 %x) {
entry:
  %cmp = icmp sgt i32 %x, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:
  %mul = mul i32 %x, 2
  ret i32 %mul

if.else:
  ret i32 0

; This block is unreachable - no predecessors
unreachable_block:
  %add = add i32 %x, 100
  ret i32 %add
}

define i32 @another_test(i32 %a, i32 %b) {
start:
  %sum = add i32 %a, %b
  ret i32 %sum

; Another unreachable block
dead_code:
  %product = mul i32 %a, %b
  ret i32 %product

; And another one
more_dead_code:
  %diff = sub i32 %a, %b
  ret i32 %diff
}
