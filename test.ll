; Simple test for dataflow analysis
; Function with multiple basic blocks and variable flow

define i32 @test_function(i32 %a, i32 %b) {
entry:
  %sum = add i32 %a, %b
  %prod = mul i32 %a, %b
  %cmp = icmp sgt i32 %sum, 10
  br i1 %cmp, label %then, label %else

then:
  %x = add i32 %sum, 5
  %y = mul i32 %prod, 2
  br label %merge

else:
  %z = sub i32 %prod, 3
  %w = add i32 %a, 1
  br label %merge

merge:
  %phi_val = phi i32 [ %x, %then ], [ %z, %else ]
  %result = add i32 %phi_val, %b
  ret i32 %result
}
