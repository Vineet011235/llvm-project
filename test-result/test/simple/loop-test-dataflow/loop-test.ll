; ModuleID = 'test/simple/loop-test.c'
source_filename = "test/simple/loop-test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__const.main.numbers = private unnamed_addr constant [5 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5], align 16

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @sum_array(ptr noundef %arr, i32 noundef %n) #0 !dbg !9 {
entry:
  %arr.addr = alloca ptr, align 8
  %n.addr = alloca i32, align 4
  %sum = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %arr, ptr %arr.addr, align 8
    #dbg_declare(ptr %arr.addr, !15, !DIExpression(), !16)
  store i32 %n, ptr %n.addr, align 4
    #dbg_declare(ptr %n.addr, !17, !DIExpression(), !18)
    #dbg_declare(ptr %sum, !19, !DIExpression(), !20)
  store i32 0, ptr %sum, align 4, !dbg !20
    #dbg_declare(ptr %i, !21, !DIExpression(), !22)
  store i32 0, ptr %i, align 4, !dbg !23
  br label %for.cond, !dbg !25

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4, !dbg !26
  %1 = load i32, ptr %n.addr, align 4, !dbg !28
  %cmp = icmp slt i32 %0, %1, !dbg !29
  br i1 %cmp, label %for.body, label %for.end, !dbg !30

for.body:                                         ; preds = %for.cond
  %2 = load ptr, ptr %arr.addr, align 8, !dbg !31
  %3 = load i32, ptr %i, align 4, !dbg !33
  %idxprom = sext i32 %3 to i64, !dbg !31
  %arrayidx = getelementptr inbounds i32, ptr %2, i64 %idxprom, !dbg !31
  %4 = load i32, ptr %arrayidx, align 4, !dbg !31
  %5 = load i32, ptr %sum, align 4, !dbg !34
  %add = add nsw i32 %5, %4, !dbg !34
  store i32 %add, ptr %sum, align 4, !dbg !34
  br label %for.inc, !dbg !35

for.inc:                                          ; preds = %for.body
  %6 = load i32, ptr %i, align 4, !dbg !36
  %inc = add nsw i32 %6, 1, !dbg !36
  store i32 %inc, ptr %i, align 4, !dbg !36
  br label %for.cond, !dbg !37, !llvm.loop !38

for.end:                                          ; preds = %for.cond
  %7 = load i32, ptr %sum, align 4, !dbg !41
  ret i32 %7, !dbg !42
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !dbg !43 {
entry:
  %retval = alloca i32, align 4
  %numbers = alloca [5 x i32], align 16
  %result = alloca i32, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %numbers, !46, !DIExpression(), !50)
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %numbers, ptr align 16 @__const.main.numbers, i64 20, i1 false), !dbg !50
    #dbg_declare(ptr %result, !51, !DIExpression(), !52)
  %arraydecay = getelementptr inbounds [5 x i32], ptr %numbers, i64 0, i64 0, !dbg !53
  %call = call i32 @sum_array(ptr noundef %arraydecay, i32 noundef 5), !dbg !54
  store i32 %call, ptr %result, align 4, !dbg !52
  %0 = load i32, ptr %result, align 4, !dbg !55
  ret i32 %0, !dbg !56
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #1

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/simple/loop-test.c", directory: "/home/decompiler/llvm/llvm-project", checksumkind: CSK_MD5, checksum: "c0eaa39852872f034d8ef88a3b82c096")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{!"clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)"}
!9 = distinct !DISubprogram(name: "sum_array", scope: !1, file: !1, line: 2, type: !10, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!10 = !DISubroutineType(types: !11)
!11 = !{!12, !13, !12}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!14 = !{}
!15 = !DILocalVariable(name: "arr", arg: 1, scope: !9, file: !1, line: 2, type: !13)
!16 = !DILocation(line: 2, column: 19, scope: !9)
!17 = !DILocalVariable(name: "n", arg: 2, scope: !9, file: !1, line: 2, type: !12)
!18 = !DILocation(line: 2, column: 30, scope: !9)
!19 = !DILocalVariable(name: "sum", scope: !9, file: !1, line: 3, type: !12)
!20 = !DILocation(line: 3, column: 9, scope: !9)
!21 = !DILocalVariable(name: "i", scope: !9, file: !1, line: 4, type: !12)
!22 = !DILocation(line: 4, column: 9, scope: !9)
!23 = !DILocation(line: 6, column: 12, scope: !24)
!24 = distinct !DILexicalBlock(scope: !9, file: !1, line: 6, column: 5)
!25 = !DILocation(line: 6, column: 10, scope: !24)
!26 = !DILocation(line: 6, column: 17, scope: !27)
!27 = distinct !DILexicalBlock(scope: !24, file: !1, line: 6, column: 5)
!28 = !DILocation(line: 6, column: 21, scope: !27)
!29 = !DILocation(line: 6, column: 19, scope: !27)
!30 = !DILocation(line: 6, column: 5, scope: !24)
!31 = !DILocation(line: 7, column: 16, scope: !32)
!32 = distinct !DILexicalBlock(scope: !27, file: !1, line: 6, column: 29)
!33 = !DILocation(line: 7, column: 20, scope: !32)
!34 = !DILocation(line: 7, column: 13, scope: !32)
!35 = !DILocation(line: 8, column: 5, scope: !32)
!36 = !DILocation(line: 6, column: 25, scope: !27)
!37 = !DILocation(line: 6, column: 5, scope: !27)
!38 = distinct !{!38, !30, !39, !40}
!39 = !DILocation(line: 8, column: 5, scope: !24)
!40 = !{!"llvm.loop.mustprogress"}
!41 = !DILocation(line: 10, column: 12, scope: !9)
!42 = !DILocation(line: 10, column: 5, scope: !9)
!43 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 13, type: !44, scopeLine: 13, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!44 = !DISubroutineType(types: !45)
!45 = !{!12}
!46 = !DILocalVariable(name: "numbers", scope: !43, file: !1, line: 14, type: !47)
!47 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 160, elements: !48)
!48 = !{!49}
!49 = !DISubrange(count: 5)
!50 = !DILocation(line: 14, column: 9, scope: !43)
!51 = !DILocalVariable(name: "result", scope: !43, file: !1, line: 15, type: !12)
!52 = !DILocation(line: 15, column: 9, scope: !43)
!53 = !DILocation(line: 15, column: 28, scope: !43)
!54 = !DILocation(line: 15, column: 18, scope: !43)
!55 = !DILocation(line: 16, column: 12, scope: !43)
!56 = !DILocation(line: 16, column: 5, scope: !43)
