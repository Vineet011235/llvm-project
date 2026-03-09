; ModuleID = 'test/simple/if-else.cc'
source_filename = "test/simple/if-else.cc"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main() #0 !dbg !9 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %x, !14, !DIExpression(), !15)
  store i32 10, ptr %x, align 4, !dbg !15
    #dbg_declare(ptr %y, !16, !DIExpression(), !17)
  %0 = load i32, ptr %x, align 4, !dbg !18
  %cmp = icmp sgt i32 %0, 5, !dbg !20
  br i1 %cmp, label %if.then, label %if.else, !dbg !20

if.then:                                          ; preds = %entry
  %1 = load i32, ptr %x, align 4, !dbg !21
  %mul = mul nsw i32 %1, 2, !dbg !23
  store i32 %mul, ptr %y, align 4, !dbg !24
  br label %if.end, !dbg !25

if.else:                                          ; preds = %entry
  %2 = load i32, ptr %x, align 4, !dbg !26
  %mul1 = mul nsw i32 10, %2, !dbg !28
  store i32 %mul1, ptr %y, align 4, !dbg !29
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret i32 0, !dbg !30
}

attributes #0 = { mustprogress noinline norecurse nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/simple/if-else.cc", directory: "/home/decompiler/llvm/llvm-project", checksumkind: CSK_MD5, checksum: "7de72b507171a00fb0a0203365ad1fde")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{!"clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git cb0eb45cdde08c584435882d6ed95085efd40816)"}
!9 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 2, type: !10, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!10 = !DISubroutineType(types: !11)
!11 = !{!12}
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{}
!14 = !DILocalVariable(name: "x", scope: !9, file: !1, line: 3, type: !12)
!15 = !DILocation(line: 3, column: 9, scope: !9)
!16 = !DILocalVariable(name: "y", scope: !9, file: !1, line: 4, type: !12)
!17 = !DILocation(line: 4, column: 9, scope: !9)
!18 = !DILocation(line: 5, column: 9, scope: !19)
!19 = distinct !DILexicalBlock(scope: !9, file: !1, line: 5, column: 9)
!20 = !DILocation(line: 5, column: 11, scope: !19)
!21 = !DILocation(line: 6, column: 13, scope: !22)
!22 = distinct !DILexicalBlock(scope: !19, file: !1, line: 5, column: 16)
!23 = !DILocation(line: 6, column: 15, scope: !22)
!24 = !DILocation(line: 6, column: 11, scope: !22)
!25 = !DILocation(line: 7, column: 5, scope: !22)
!26 = !DILocation(line: 8, column: 18, scope: !27)
!27 = distinct !DILexicalBlock(scope: !19, file: !1, line: 7, column: 12)
!28 = !DILocation(line: 8, column: 16, scope: !27)
!29 = !DILocation(line: 8, column: 11, scope: !27)
!30 = !DILocation(line: 10, column: 5, scope: !9)
