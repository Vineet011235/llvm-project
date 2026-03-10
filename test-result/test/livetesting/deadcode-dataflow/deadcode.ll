; ModuleID = 'test/livetesting/deadcode.cc'
source_filename = "test/livetesting/deadcode.cc"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z14dead_code_testii(i32 noundef %a, i32 noundef %b) #0 !dbg !9 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  %dead_val = alloca i32, align 4
  %c = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
    #dbg_declare(ptr %a.addr, !14, !DIExpression(), !15)
  store i32 %b, ptr %b.addr, align 4
    #dbg_declare(ptr %b.addr, !16, !DIExpression(), !17)
    #dbg_declare(ptr %dead_val, !18, !DIExpression(), !19)
  %0 = load i32, ptr %a.addr, align 4, !dbg !20
  %mul = mul nsw i32 %0, 100, !dbg !21
  store i32 %mul, ptr %dead_val, align 4, !dbg !19
    #dbg_declare(ptr %c, !22, !DIExpression(), !23)
  %1 = load i32, ptr %b.addr, align 4, !dbg !24
  %add = add nsw i32 %1, 5, !dbg !25
  store i32 %add, ptr %c, align 4, !dbg !23
  %2 = load i32, ptr %c, align 4, !dbg !26
  ret i32 %2, !dbg !27
}

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main() #1 !dbg !28 {
entry:
  %retval = alloca i32, align 4
  %result = alloca i32, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %result, !31, !DIExpression(), !32)
  %call = call noundef i32 @_Z14dead_code_testii(i32 noundef 5, i32 noundef 10), !dbg !33
  store i32 %call, ptr %result, align 4, !dbg !32
  ret i32 0, !dbg !34
}

attributes #0 = { mustprogress noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress noinline norecurse nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/livetesting/deadcode.cc", directory: "/home/decompiler/llvm/llvm-project", checksumkind: CSK_MD5, checksum: "301896e0f555f82bc78e9698d517a402")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{!"clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)"}
!9 = distinct !DISubprogram(name: "dead_code_test", linkageName: "_Z14dead_code_testii", scope: !1, file: !1, line: 2, type: !10, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!10 = !DISubroutineType(types: !11)
!11 = !{!12, !12, !12}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{}
!14 = !DILocalVariable(name: "a", arg: 1, scope: !9, file: !1, line: 2, type: !12)
!15 = !DILocation(line: 2, column: 24, scope: !9)
!16 = !DILocalVariable(name: "b", arg: 2, scope: !9, file: !1, line: 2, type: !12)
!17 = !DILocation(line: 2, column: 31, scope: !9)
!18 = !DILocalVariable(name: "dead_val", scope: !9, file: !1, line: 4, type: !12)
!19 = !DILocation(line: 4, column: 9, scope: !9)
!20 = !DILocation(line: 4, column: 20, scope: !9)
!21 = !DILocation(line: 4, column: 22, scope: !9)
!22 = !DILocalVariable(name: "c", scope: !9, file: !1, line: 7, type: !12)
!23 = !DILocation(line: 7, column: 9, scope: !9)
!24 = !DILocation(line: 7, column: 13, scope: !9)
!25 = !DILocation(line: 7, column: 15, scope: !9)
!26 = !DILocation(line: 9, column: 12, scope: !9)
!27 = !DILocation(line: 9, column: 5, scope: !9)
!28 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 12, type: !29, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!29 = !DISubroutineType(types: !30)
!30 = !{!12}
!31 = !DILocalVariable(name: "result", scope: !28, file: !1, line: 13, type: !12)
!32 = !DILocation(line: 13, column: 9, scope: !28)
!33 = !DILocation(line: 13, column: 18, scope: !28)
!34 = !DILocation(line: 14, column: 5, scope: !28)
