; ModuleID = 'test/livetesting/loop.cc'
source_filename = "test/livetesting/loop.cc"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z16loop_stress_testii(i32 noundef %a, i32 noundef %b) #0 !dbg !9 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  %sum = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
    #dbg_declare(ptr %a.addr, !14, !DIExpression(), !15)
  store i32 %b, ptr %b.addr, align 4
    #dbg_declare(ptr %b.addr, !16, !DIExpression(), !17)
    #dbg_declare(ptr %sum, !18, !DIExpression(), !19)
  %0 = load i32, ptr %a.addr, align 4, !dbg !20
  store i32 %0, ptr %sum, align 4, !dbg !19
    #dbg_declare(ptr %i, !21, !DIExpression(), !22)
  store i32 0, ptr %i, align 4, !dbg !22
  br label %while.cond, !dbg !23

while.cond:                                       ; preds = %while.body, %entry
  %1 = load i32, ptr %i, align 4, !dbg !24
  %2 = load i32, ptr %b.addr, align 4, !dbg !25
  %cmp = icmp slt i32 %1, %2, !dbg !26
  br i1 %cmp, label %while.body, label %while.end, !dbg !23

while.body:                                       ; preds = %while.cond
  %3 = load i32, ptr %sum, align 4, !dbg !27
  %4 = load i32, ptr %i, align 4, !dbg !29
  %add = add nsw i32 %3, %4, !dbg !30
  store i32 %add, ptr %sum, align 4, !dbg !31
  %5 = load i32, ptr %i, align 4, !dbg !32
  %inc = add nsw i32 %5, 1, !dbg !32
  store i32 %inc, ptr %i, align 4, !dbg !32
  br label %while.cond, !dbg !23, !llvm.loop !33

while.end:                                        ; preds = %while.cond
  %6 = load i32, ptr %sum, align 4, !dbg !36
  ret i32 %6, !dbg !37
}

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main() #1 !dbg !38 {
entry:
  %retval = alloca i32, align 4
  %result = alloca i32, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %result, !41, !DIExpression(), !42)
  %call = call noundef i32 @_Z16loop_stress_testii(i32 noundef 5, i32 noundef 10), !dbg !43
  store i32 %call, ptr %result, align 4, !dbg !42
  ret i32 0, !dbg !44
}

attributes #0 = { mustprogress noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress noinline norecurse nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/livetesting/loop.cc", directory: "/home/decompiler/llvm/llvm-project", checksumkind: CSK_MD5, checksum: "4df13544f864b01ede83094a3c66e83e")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{!"clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)"}
!9 = distinct !DISubprogram(name: "loop_stress_test", linkageName: "_Z16loop_stress_testii", scope: !1, file: !1, line: 2, type: !10, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!10 = !DISubroutineType(types: !11)
!11 = !{!12, !12, !12}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{}
!14 = !DILocalVariable(name: "a", arg: 1, scope: !9, file: !1, line: 2, type: !12)
!15 = !DILocation(line: 2, column: 26, scope: !9)
!16 = !DILocalVariable(name: "b", arg: 2, scope: !9, file: !1, line: 2, type: !12)
!17 = !DILocation(line: 2, column: 33, scope: !9)
!18 = !DILocalVariable(name: "sum", scope: !9, file: !1, line: 3, type: !12)
!19 = !DILocation(line: 3, column: 9, scope: !9)
!20 = !DILocation(line: 3, column: 15, scope: !9)
!21 = !DILocalVariable(name: "i", scope: !9, file: !1, line: 4, type: !12)
!22 = !DILocation(line: 4, column: 9, scope: !9)
!23 = !DILocation(line: 7, column: 5, scope: !9)
!24 = !DILocation(line: 7, column: 12, scope: !9)
!25 = !DILocation(line: 7, column: 16, scope: !9)
!26 = !DILocation(line: 7, column: 14, scope: !9)
!27 = !DILocation(line: 8, column: 15, scope: !28)
!28 = distinct !DILexicalBlock(scope: !9, file: !1, line: 7, column: 19)
!29 = !DILocation(line: 8, column: 21, scope: !28)
!30 = !DILocation(line: 8, column: 19, scope: !28)
!31 = !DILocation(line: 8, column: 13, scope: !28)
!32 = !DILocation(line: 9, column: 10, scope: !28)
!33 = distinct !{!33, !23, !34, !35}
!34 = !DILocation(line: 10, column: 5, scope: !9)
!35 = !{!"llvm.loop.mustprogress"}
!36 = !DILocation(line: 13, column: 12, scope: !9)
!37 = !DILocation(line: 13, column: 5, scope: !9)
!38 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 16, type: !39, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!39 = !DISubroutineType(types: !40)
!40 = !{!12}
!41 = !DILocalVariable(name: "result", scope: !38, file: !1, line: 17, type: !12)
!42 = !DILocation(line: 17, column: 9, scope: !38)
!43 = !DILocation(line: 17, column: 18, scope: !38)
!44 = !DILocation(line: 18, column: 5, scope: !38)
