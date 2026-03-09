; ModuleID = 'test/simple/multiple-paths.c'
source_filename = "test/simple/multiple-paths.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @calculate(i32 noundef %x, i32 noundef %y, i32 noundef %mode) #0 !dbg !9 {
entry:
  %x.addr = alloca i32, align 4
  %y.addr = alloca i32, align 4
  %mode.addr = alloca i32, align 4
  %result = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
    #dbg_declare(ptr %x.addr, !14, !DIExpression(), !15)
  store i32 %y, ptr %y.addr, align 4
    #dbg_declare(ptr %y.addr, !16, !DIExpression(), !17)
  store i32 %mode, ptr %mode.addr, align 4
    #dbg_declare(ptr %mode.addr, !18, !DIExpression(), !19)
    #dbg_declare(ptr %result, !20, !DIExpression(), !21)
  %0 = load i32, ptr %mode.addr, align 4, !dbg !22
  %cmp = icmp eq i32 %0, 0, !dbg !24
  br i1 %cmp, label %if.then, label %if.else, !dbg !24

if.then:                                          ; preds = %entry
  %1 = load i32, ptr %x.addr, align 4, !dbg !25
  %2 = load i32, ptr %y.addr, align 4, !dbg !27
  %add = add nsw i32 %1, %2, !dbg !28
  store i32 %add, ptr %result, align 4, !dbg !29
  br label %if.end8, !dbg !30

if.else:                                          ; preds = %entry
  %3 = load i32, ptr %mode.addr, align 4, !dbg !31
  %cmp1 = icmp eq i32 %3, 1, !dbg !33
  br i1 %cmp1, label %if.then2, label %if.else3, !dbg !33

if.then2:                                         ; preds = %if.else
  %4 = load i32, ptr %x.addr, align 4, !dbg !34
  %5 = load i32, ptr %y.addr, align 4, !dbg !36
  %sub = sub nsw i32 %4, %5, !dbg !37
  store i32 %sub, ptr %result, align 4, !dbg !38
  br label %if.end7, !dbg !39

if.else3:                                         ; preds = %if.else
  %6 = load i32, ptr %mode.addr, align 4, !dbg !40
  %cmp4 = icmp eq i32 %6, 2, !dbg !42
  br i1 %cmp4, label %if.then5, label %if.else6, !dbg !42

if.then5:                                         ; preds = %if.else3
  %7 = load i32, ptr %x.addr, align 4, !dbg !43
  %8 = load i32, ptr %y.addr, align 4, !dbg !45
  %mul = mul nsw i32 %7, %8, !dbg !46
  store i32 %mul, ptr %result, align 4, !dbg !47
  br label %if.end, !dbg !48

if.else6:                                         ; preds = %if.else3
  store i32 0, ptr %result, align 4, !dbg !49
  br label %if.end

if.end:                                           ; preds = %if.else6, %if.then5
  br label %if.end7

if.end7:                                          ; preds = %if.end, %if.then2
  br label %if.end8

if.end8:                                          ; preds = %if.end7, %if.then
  %9 = load i32, ptr %result, align 4, !dbg !51
  ret i32 %9, !dbg !52
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !dbg !53 {
entry:
  %retval = alloca i32, align 4
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %a, !56, !DIExpression(), !57)
  %call = call i32 @calculate(i32 noundef 10, i32 noundef 5, i32 noundef 0), !dbg !58
  store i32 %call, ptr %a, align 4, !dbg !57
    #dbg_declare(ptr %b, !59, !DIExpression(), !60)
  %call1 = call i32 @calculate(i32 noundef 10, i32 noundef 5, i32 noundef 1), !dbg !61
  store i32 %call1, ptr %b, align 4, !dbg !60
    #dbg_declare(ptr %c, !62, !DIExpression(), !63)
  %call2 = call i32 @calculate(i32 noundef 10, i32 noundef 5, i32 noundef 2), !dbg !64
  store i32 %call2, ptr %c, align 4, !dbg !63
  %0 = load i32, ptr %a, align 4, !dbg !65
  %1 = load i32, ptr %b, align 4, !dbg !66
  %add = add nsw i32 %0, %1, !dbg !67
  %2 = load i32, ptr %c, align 4, !dbg !68
  %add3 = add nsw i32 %add, %2, !dbg !69
  ret i32 %add3, !dbg !70
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/simple/multiple-paths.c", directory: "/home/decompiler/llvm/llvm-project", checksumkind: CSK_MD5, checksum: "96e4609043515b80c65c55cb805585b1")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{!"clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)"}
!9 = distinct !DISubprogram(name: "calculate", scope: !1, file: !1, line: 2, type: !10, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!10 = !DISubroutineType(types: !11)
!11 = !{!12, !12, !12, !12}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{}
!14 = !DILocalVariable(name: "x", arg: 1, scope: !9, file: !1, line: 2, type: !12)
!15 = !DILocation(line: 2, column: 19, scope: !9)
!16 = !DILocalVariable(name: "y", arg: 2, scope: !9, file: !1, line: 2, type: !12)
!17 = !DILocation(line: 2, column: 26, scope: !9)
!18 = !DILocalVariable(name: "mode", arg: 3, scope: !9, file: !1, line: 2, type: !12)
!19 = !DILocation(line: 2, column: 33, scope: !9)
!20 = !DILocalVariable(name: "result", scope: !9, file: !1, line: 3, type: !12)
!21 = !DILocation(line: 3, column: 9, scope: !9)
!22 = !DILocation(line: 5, column: 9, scope: !23)
!23 = distinct !DILexicalBlock(scope: !9, file: !1, line: 5, column: 9)
!24 = !DILocation(line: 5, column: 14, scope: !23)
!25 = !DILocation(line: 6, column: 18, scope: !26)
!26 = distinct !DILexicalBlock(scope: !23, file: !1, line: 5, column: 20)
!27 = !DILocation(line: 6, column: 22, scope: !26)
!28 = !DILocation(line: 6, column: 20, scope: !26)
!29 = !DILocation(line: 6, column: 16, scope: !26)
!30 = !DILocation(line: 7, column: 5, scope: !26)
!31 = !DILocation(line: 7, column: 16, scope: !32)
!32 = distinct !DILexicalBlock(scope: !23, file: !1, line: 7, column: 16)
!33 = !DILocation(line: 7, column: 21, scope: !32)
!34 = !DILocation(line: 8, column: 18, scope: !35)
!35 = distinct !DILexicalBlock(scope: !32, file: !1, line: 7, column: 27)
!36 = !DILocation(line: 8, column: 22, scope: !35)
!37 = !DILocation(line: 8, column: 20, scope: !35)
!38 = !DILocation(line: 8, column: 16, scope: !35)
!39 = !DILocation(line: 9, column: 5, scope: !35)
!40 = !DILocation(line: 9, column: 16, scope: !41)
!41 = distinct !DILexicalBlock(scope: !32, file: !1, line: 9, column: 16)
!42 = !DILocation(line: 9, column: 21, scope: !41)
!43 = !DILocation(line: 10, column: 18, scope: !44)
!44 = distinct !DILexicalBlock(scope: !41, file: !1, line: 9, column: 27)
!45 = !DILocation(line: 10, column: 22, scope: !44)
!46 = !DILocation(line: 10, column: 20, scope: !44)
!47 = !DILocation(line: 10, column: 16, scope: !44)
!48 = !DILocation(line: 11, column: 5, scope: !44)
!49 = !DILocation(line: 12, column: 16, scope: !50)
!50 = distinct !DILexicalBlock(scope: !41, file: !1, line: 11, column: 12)
!51 = !DILocation(line: 15, column: 12, scope: !9)
!52 = !DILocation(line: 15, column: 5, scope: !9)
!53 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 18, type: !54, scopeLine: 18, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!54 = !DISubroutineType(types: !55)
!55 = !{!12}
!56 = !DILocalVariable(name: "a", scope: !53, file: !1, line: 19, type: !12)
!57 = !DILocation(line: 19, column: 9, scope: !53)
!58 = !DILocation(line: 19, column: 13, scope: !53)
!59 = !DILocalVariable(name: "b", scope: !53, file: !1, line: 20, type: !12)
!60 = !DILocation(line: 20, column: 9, scope: !53)
!61 = !DILocation(line: 20, column: 13, scope: !53)
!62 = !DILocalVariable(name: "c", scope: !53, file: !1, line: 21, type: !12)
!63 = !DILocation(line: 21, column: 9, scope: !53)
!64 = !DILocation(line: 21, column: 13, scope: !53)
!65 = !DILocation(line: 23, column: 12, scope: !53)
!66 = !DILocation(line: 23, column: 16, scope: !53)
!67 = !DILocation(line: 23, column: 14, scope: !53)
!68 = !DILocation(line: 23, column: 20, scope: !53)
!69 = !DILocation(line: 23, column: 18, scope: !53)
!70 = !DILocation(line: 23, column: 5, scope: !53)
