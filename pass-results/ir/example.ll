; ModuleID = 'example.c'
source_filename = "example.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__const.main.test_values = private unnamed_addr constant [6 x i32] [i32 0, i32 1, i32 5, i32 10, i32 15, i32 20], align 16
@.str.1 = private unnamed_addr constant [20 x i8] c"fibonacci(%d) = %d\0A\00", align 1, !dbg !0
@str = private unnamed_addr constant [19 x i8] c"Fibonacci Results:\00", align 1

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local i32 @fibonacci(i32 noundef %n) local_unnamed_addr #0 !dbg !23 {
entry:
    #dbg_value(i32 %n, !28, !DIExpression(), !36)
  %cmp = icmp slt i32 %n, 2, !dbg !37
  br i1 %cmp, label %return, label %for.body, !dbg !39

for.body:                                         ; preds = %entry, %for.body
  %i.09 = phi i32 [ %inc, %for.body ], [ 2, %entry ]
  %curr.08 = phi i32 [ %add, %for.body ], [ 1, %entry ]
  %prev.07 = phi i32 [ %curr.08, %for.body ], [ 0, %entry ]
    #dbg_value(i32 %i.09, !31, !DIExpression(), !40)
    #dbg_value(i32 %curr.08, !30, !DIExpression(), !36)
    #dbg_value(i32 %prev.07, !29, !DIExpression(), !36)
  %add = add nsw i32 %curr.08, %prev.07, !dbg !41
    #dbg_value(i32 %add, !33, !DIExpression(), !42)
    #dbg_value(i32 %curr.08, !29, !DIExpression(), !36)
    #dbg_value(i32 %add, !30, !DIExpression(), !36)
  %inc = add nuw i32 %i.09, 1, !dbg !43
    #dbg_value(i32 %inc, !31, !DIExpression(), !40)
  %exitcond.not = icmp eq i32 %i.09, %n, !dbg !44
  br i1 %exitcond.not, label %return, label %for.body, !dbg !45, !llvm.loop !46

return:                                           ; preds = %for.body, %entry
  %retval.0 = phi i32 [ %n, %entry ], [ %add, %for.body ], !dbg !36
  ret i32 %retval.0, !dbg !51
}

; Function Attrs: nofree nounwind uwtable
define dso_local noundef i32 @main() local_unnamed_addr #1 !dbg !52 {
entry:
    #dbg_assign(i1 poison, !56, !DIExpression(), !66, ptr @__const.main.test_values, !DIExpression(), !67)
    #dbg_assign(i1 poison, !56, !DIExpression(), !68, ptr @__const.main.test_values, !DIExpression(), !67)
    #dbg_value(i32 6, !60, !DIExpression(), !67)
  %puts = tail call i32 @puts(ptr nonnull dereferenceable(1) @str), !dbg !69
    #dbg_value(i32 0, !61, !DIExpression(), !70)
  br label %for.body, !dbg !71

for.cond.cleanup:                                 ; preds = %fibonacci.exit
  ret i32 0, !dbg !72

for.body:                                         ; preds = %entry, %fibonacci.exit
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %fibonacci.exit ]
    #dbg_value(i64 %indvars.iv, !61, !DIExpression(), !70)
  %arrayidx = getelementptr inbounds nuw i32, ptr @__const.main.test_values, i64 %indvars.iv, !dbg !73
  %0 = load i32, ptr %arrayidx, align 4, !dbg !74, !tbaa !19
    #dbg_value(i32 %0, !63, !DIExpression(), !75)
    #dbg_value(i32 %0, !28, !DIExpression(), !76)
  %cmp.i = icmp samesign ult i64 %indvars.iv, 2, !dbg !78
  br i1 %cmp.i, label %fibonacci.exit, label %for.body.i, !dbg !79

for.body.i:                                       ; preds = %for.body, %for.body.i
  %i.09.i = phi i32 [ %inc.i, %for.body.i ], [ 2, %for.body ]
  %curr.08.i = phi i32 [ %add.i, %for.body.i ], [ 1, %for.body ]
  %prev.07.i = phi i32 [ %curr.08.i, %for.body.i ], [ 0, %for.body ]
    #dbg_value(i32 %i.09.i, !31, !DIExpression(), !80)
    #dbg_value(i32 %curr.08.i, !30, !DIExpression(), !76)
    #dbg_value(i32 %prev.07.i, !29, !DIExpression(), !76)
  %add.i = add nsw i32 %prev.07.i, %curr.08.i, !dbg !81
    #dbg_value(i32 %add.i, !33, !DIExpression(), !82)
    #dbg_value(i32 %curr.08.i, !29, !DIExpression(), !76)
    #dbg_value(i32 %add.i, !30, !DIExpression(), !76)
  %inc.i = add nuw i32 %i.09.i, 1, !dbg !83
    #dbg_value(i32 %inc.i, !31, !DIExpression(), !80)
  %exitcond.not.i = icmp eq i32 %i.09.i, %0, !dbg !84
  br i1 %exitcond.not.i, label %fibonacci.exit, label %for.body.i, !dbg !85, !llvm.loop !86

fibonacci.exit:                                   ; preds = %for.body.i, %for.body
  %retval.0.i = phi i32 [ %0, %for.body ], [ %add.i, %for.body.i ], !dbg !76
  %call2 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.1, i32 noundef %0, i32 noundef %retval.0.i), !dbg !89
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !90
    #dbg_value(i64 %indvars.iv.next, !61, !DIExpression(), !70)
  %exitcond.not = icmp eq i64 %indvars.iv.next, 6, !dbg !91
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !dbg !92, !llvm.loop !93
}

; Function Attrs: nofree nounwind
declare !dbg !96 noundef i32 @printf(ptr noundef readonly captures(none), ...) local_unnamed_addr #2

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr noundef readonly captures(none)) local_unnamed_addr #3

attributes #0 = { nofree norecurse nosync nounwind memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nofree nounwind }

!llvm.dbg.cu = !{!7}
!llvm.module.flags = !{!11, !12, !13, !14, !15, !16, !17}
!llvm.ident = !{!18}
!llvm.errno.tbaa = !{!19}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 25, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "example.c", directory: "/home/decompiler/llvm/llvm-project", checksumkind: CSK_MD5, checksum: "cd1a00b62023d2fd0d00610658050261")
!3 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 160, elements: !5)
!4 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!5 = !{!6}
!6 = !DISubrange(count: 20)
!7 = distinct !DICompileUnit(language: DW_LANG_C11, file: !2, producer: "clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git 8afba560323005dd570a5646f2f1258124388c88)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, globals: !8, splitDebugInlining: false, nameTableKind: None)
!8 = !{!9, !0}
!9 = !DIGlobalVariableExpression(var: !10, expr: !DIExpression())
!10 = distinct !DIGlobalVariable(scope: null, file: !2, line: 22, type: !3, isLocal: true, isDefinition: true)
!11 = !{i32 7, !"Dwarf Version", i32 5}
!12 = !{i32 2, !"Debug Info Version", i32 3}
!13 = !{i32 1, !"wchar_size", i32 4}
!14 = !{i32 8, !"PIC Level", i32 2}
!15 = !{i32 7, !"PIE Level", i32 2}
!16 = !{i32 7, !"uwtable", i32 2}
!17 = !{i32 7, !"debug-info-assignment-tracking", i1 true}
!18 = !{!"clang version 23.0.0git (git@github.com:Vineet011235/llvm-project.git 8afba560323005dd570a5646f2f1258124388c88)"}
!19 = !{!20, !20, i64 0}
!20 = !{!"int", !21, i64 0}
!21 = !{!"omnipotent char", !22, i64 0}
!22 = !{!"Simple C/C++ TBAA"}
!23 = distinct !DISubprogram(name: "fibonacci", scope: !2, file: !2, line: 5, type: !24, scopeLine: 5, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !27, keyInstructions: true)
!24 = !DISubroutineType(types: !25)
!25 = !{!26, !26}
!26 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!27 = !{!28, !29, !30, !31, !33}
!28 = !DILocalVariable(name: "n", arg: 1, scope: !23, file: !2, line: 5, type: !26)
!29 = !DILocalVariable(name: "prev", scope: !23, file: !2, line: 9, type: !26)
!30 = !DILocalVariable(name: "curr", scope: !23, file: !2, line: 9, type: !26)
!31 = !DILocalVariable(name: "i", scope: !32, file: !2, line: 10, type: !26)
!32 = distinct !DILexicalBlock(scope: !23, file: !2, line: 10, column: 5)
!33 = !DILocalVariable(name: "next", scope: !34, file: !2, line: 11, type: !26)
!34 = distinct !DILexicalBlock(scope: !35, file: !2, line: 10, column: 34)
!35 = distinct !DILexicalBlock(scope: !32, file: !2, line: 10, column: 5)
!36 = !DILocation(line: 0, scope: !23)
!37 = !DILocation(line: 6, column: 11, scope: !38, atomGroup: 1, atomRank: 2)
!38 = distinct !DILexicalBlock(scope: !23, file: !2, line: 6, column: 9)
!39 = !DILocation(line: 6, column: 11, scope: !38, atomGroup: 1, atomRank: 1)
!40 = !DILocation(line: 0, scope: !32)
!41 = !DILocation(line: 11, column: 25, scope: !34, atomGroup: 8, atomRank: 2)
!42 = !DILocation(line: 0, scope: !34)
!43 = !DILocation(line: 10, column: 30, scope: !35, atomGroup: 11, atomRank: 2)
!44 = !DILocation(line: 10, column: 23, scope: !35, atomGroup: 6, atomRank: 1)
!45 = !DILocation(line: 10, column: 5, scope: !32, atomGroup: 7, atomRank: 1)
!46 = distinct !{!46, !47, !48, !49, !50}
!47 = !DILocation(line: 10, column: 5, scope: !32)
!48 = !DILocation(line: 14, column: 5, scope: !32)
!49 = !{!"llvm.loop.mustprogress"}
!50 = !{!"llvm.loop.unroll.disable"}
!51 = !DILocation(line: 16, column: 1, scope: !23, atomGroup: 14, atomRank: 1)
!52 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 18, type: !53, scopeLine: 18, flags: DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !7, retainedNodes: !55, keyInstructions: true)
!53 = !DISubroutineType(types: !54)
!54 = !{!26}
!55 = !{!56, !60, !61, !63}
!56 = !DILocalVariable(name: "test_values", scope: !52, file: !2, line: 19, type: !57)
!57 = !DICompositeType(tag: DW_TAG_array_type, baseType: !26, size: 192, elements: !58)
!58 = !{!59}
!59 = !DISubrange(count: 6)
!60 = !DILocalVariable(name: "num_tests", scope: !52, file: !2, line: 20, type: !26)
!61 = !DILocalVariable(name: "i", scope: !62, file: !2, line: 23, type: !26)
!62 = distinct !DILexicalBlock(scope: !52, file: !2, line: 23, column: 5)
!63 = !DILocalVariable(name: "n", scope: !64, file: !2, line: 24, type: !26)
!64 = distinct !DILexicalBlock(scope: !65, file: !2, line: 23, column: 41)
!65 = distinct !DILexicalBlock(scope: !62, file: !2, line: 23, column: 5)
!66 = distinct !DIAssignID()
!67 = !DILocation(line: 0, scope: !52)
!68 = distinct !DIAssignID()
!69 = !DILocation(line: 22, column: 5, scope: !52)
!70 = !DILocation(line: 0, scope: !62)
!71 = !DILocation(line: 23, column: 5, scope: !62, atomGroup: 18, atomRank: 1)
!72 = !DILocation(line: 28, column: 5, scope: !52, atomGroup: 10, atomRank: 1)
!73 = !DILocation(line: 24, column: 17, scope: !64)
!74 = !DILocation(line: 24, column: 17, scope: !64, atomGroup: 6, atomRank: 2)
!75 = !DILocation(line: 0, scope: !64)
!76 = !DILocation(line: 0, scope: !23, inlinedAt: !77)
!77 = distinct !DILocation(line: 25, column: 43, scope: !64)
!78 = !DILocation(line: 6, column: 11, scope: !38, inlinedAt: !77, atomGroup: 1, atomRank: 2)
!79 = !DILocation(line: 6, column: 11, scope: !38, inlinedAt: !77, atomGroup: 1, atomRank: 1)
!80 = !DILocation(line: 0, scope: !32, inlinedAt: !77)
!81 = !DILocation(line: 11, column: 25, scope: !34, inlinedAt: !77, atomGroup: 8, atomRank: 2)
!82 = !DILocation(line: 0, scope: !34, inlinedAt: !77)
!83 = !DILocation(line: 10, column: 30, scope: !35, inlinedAt: !77, atomGroup: 11, atomRank: 2)
!84 = !DILocation(line: 10, column: 23, scope: !35, inlinedAt: !77, atomGroup: 6, atomRank: 1)
!85 = !DILocation(line: 10, column: 5, scope: !32, inlinedAt: !77, atomGroup: 7, atomRank: 1)
!86 = distinct !{!86, !87, !88, !49, !50}
!87 = !DILocation(line: 10, column: 5, scope: !32, inlinedAt: !77)
!88 = !DILocation(line: 14, column: 5, scope: !32, inlinedAt: !77)
!89 = !DILocation(line: 25, column: 9, scope: !64)
!90 = !DILocation(line: 23, column: 37, scope: !65, atomGroup: 7, atomRank: 2)
!91 = !DILocation(line: 23, column: 23, scope: !65, atomGroup: 4, atomRank: 1)
!92 = !DILocation(line: 23, column: 5, scope: !62, atomGroup: 5, atomRank: 1)
!93 = distinct !{!93, !94, !95, !49, !50}
!94 = !DILocation(line: 23, column: 5, scope: !62)
!95 = !DILocation(line: 26, column: 5, scope: !62)
!96 = !DISubprogram(name: "printf", scope: !97, file: !97, line: 363, type: !98, flags: DIFlagPrototyped, spFlags: DISPFlagOptimized)
!97 = !DIFile(filename: "/usr/include/stdio.h", directory: "", checksumkind: CSK_MD5, checksum: "1e435c46987a169d9f9186f63a512303")
!98 = !DISubroutineType(types: !99)
!99 = !{!26, !100, null}
!100 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !101)
!101 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !102, size: 64)
!102 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !4)
