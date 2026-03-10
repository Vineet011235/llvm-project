; ModuleID = 'test/livetesting/earlyExit.cc'
source_filename = "test/livetesting/earlyExit.cc"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z15early_exit_testiii(i32 noundef %condition, i32 noundef %x, i32 noundef %y) #0 !dbg !9 {
entry:
  %retval = alloca i32, align 4
  %condition.addr = alloca i32, align 4
  %x.addr = alloca i32, align 4
  %y.addr = alloca i32, align 4
  %result = alloca i32, align 4
  store i32 %condition, ptr %condition.addr, align 4
    #dbg_declare(ptr %condition.addr, !14, !DIExpression(), !15)
  store i32 %x, ptr %x.addr, align 4
    #dbg_declare(ptr %x.addr, !16, !DIExpression(), !17)
  store i32 %y, ptr %y.addr, align 4
    #dbg_declare(ptr %y.addr, !18, !DIExpression(), !19)
    #dbg_declare(ptr %result, !20, !DIExpression(), !21)
  store i32 0, ptr %result, align 4, !dbg !21
  %0 = load i32, ptr %condition.addr, align 4, !dbg !22
  %cmp = icmp slt i32 %0, 0, !dbg !24
  br i1 %cmp, label %if.then, label %if.end, !dbg !24

if.then:                                          ; preds = %entry
  store i32 -1, ptr %retval, align 4, !dbg !25
  br label %return, !dbg !25

if.end:                                           ; preds = %entry
  %1 = load i32, ptr %condition.addr, align 4, !dbg !27
  %cmp1 = icmp eq i32 %1, 0, !dbg !29
  br i1 %cmp1, label %if.then2, label %if.else, !dbg !29

if.then2:                                         ; preds = %if.end
  %2 = load i32, ptr %x.addr, align 4, !dbg !30
  %add = add nsw i32 %2, 10, !dbg !32
  store i32 %add, ptr %result, align 4, !dbg !33
  br label %if.end3, !dbg !34

if.else:                                          ; preds = %if.end
  %3 = load i32, ptr %x.addr, align 4, !dbg !35
  %4 = load i32, ptr %y.addr, align 4, !dbg !37
  %mul = mul nsw i32 %3, %4, !dbg !38
  store i32 %mul, ptr %result, align 4, !dbg !39
  br label %if.end3

if.end3:                                          ; preds = %if.else, %if.then2
  %5 = load i32, ptr %result, align 4, !dbg !40
  store i32 %5, ptr %retval, align 4, !dbg !41
  br label %return, !dbg !41

return:                                           ; preds = %if.end3, %if.then
  %6 = load i32, ptr %retval, align 4, !dbg !42
  ret i32 %6, !dbg !42
}

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main() #1 !dbg !43 {
entry:
  %retval = alloca i32, align 4
  %result = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %result, !46, !DIExpression(), !47)
  %call = call noundef i32 @_Z15early_exit_testiii(i32 noundef -5, i32 noundef 10, i32 noundef 20), !dbg !48
  store i32 %call, ptr %result, align 4, !dbg !47
    #dbg_declare(ptr %x, !49, !DIExpression(), !50)
  %0 = load i32, ptr %result, align 4, !dbg !51
  store i32 %0, ptr %x, align 4, !dbg !50
  ret i32 0, !dbg !52
}

attributes #0 = { mustprogress noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress noinline norecurse nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/livetesting/earlyExit.cc", directory: "/home/decompiler/llvm/llvm-project", checksumkind: CSK_MD5, checksum: "fee5ee5823094c831ba3957c306871fc")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{!"clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)"}
!9 = distinct !DISubprogram(name: "early_exit_test", linkageName: "_Z15early_exit_testiii", scope: !1, file: !1, line: 1, type: !10, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!10 = !DISubroutineType(types: !11)
!11 = !{!12, !12, !12, !12}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{}
!14 = !DILocalVariable(name: "condition", arg: 1, scope: !9, file: !1, line: 1, type: !12)
!15 = !DILocation(line: 1, column: 25, scope: !9)
!16 = !DILocalVariable(name: "x", arg: 2, scope: !9, file: !1, line: 1, type: !12)
!17 = !DILocation(line: 1, column: 40, scope: !9)
!18 = !DILocalVariable(name: "y", arg: 3, scope: !9, file: !1, line: 1, type: !12)
!19 = !DILocation(line: 1, column: 47, scope: !9)
!20 = !DILocalVariable(name: "result", scope: !9, file: !1, line: 2, type: !12)
!21 = !DILocation(line: 2, column: 9, scope: !9)
!22 = !DILocation(line: 4, column: 9, scope: !23)
!23 = distinct !DILexicalBlock(scope: !9, file: !1, line: 4, column: 9)
!24 = !DILocation(line: 4, column: 19, scope: !23)
!25 = !DILocation(line: 7, column: 9, scope: !26)
!26 = distinct !DILexicalBlock(scope: !23, file: !1, line: 4, column: 24)
!27 = !DILocation(line: 11, column: 9, scope: !28)
!28 = distinct !DILexicalBlock(scope: !9, file: !1, line: 11, column: 9)
!29 = !DILocation(line: 11, column: 19, scope: !28)
!30 = !DILocation(line: 12, column: 18, scope: !31)
!31 = distinct !DILexicalBlock(scope: !28, file: !1, line: 11, column: 25)
!32 = !DILocation(line: 12, column: 20, scope: !31)
!33 = !DILocation(line: 12, column: 16, scope: !31)
!34 = !DILocation(line: 13, column: 5, scope: !31)
!35 = !DILocation(line: 14, column: 18, scope: !36)
!36 = distinct !DILexicalBlock(scope: !28, file: !1, line: 13, column: 12)
!37 = !DILocation(line: 14, column: 22, scope: !36)
!38 = !DILocation(line: 14, column: 20, scope: !36)
!39 = !DILocation(line: 14, column: 16, scope: !36)
!40 = !DILocation(line: 18, column: 12, scope: !9)
!41 = !DILocation(line: 18, column: 5, scope: !9)
!42 = !DILocation(line: 19, column: 1, scope: !9)
!43 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 21, type: !44, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!44 = !DISubroutineType(types: !45)
!45 = !{!12}
!46 = !DILocalVariable(name: "result", scope: !43, file: !1, line: 22, type: !12)
!47 = !DILocation(line: 22, column: 9, scope: !43)
!48 = !DILocation(line: 22, column: 18, scope: !43)
!49 = !DILocalVariable(name: "x", scope: !43, file: !1, line: 23, type: !12)
!50 = !DILocation(line: 23, column: 9, scope: !43)
!51 = !DILocation(line: 23, column: 13, scope: !43)
!52 = !DILocation(line: 24, column: 5, scope: !43)
