(*
---
title: Kernighan’s Bitcount
tags: [bitcount, popcount, integer]
summary: Count set bits by clearing the lowest 1-bit.
slug: kernighan-bitcount
---
*)

Definition same_cardinality (A:Type) (B:Type) :=
  { f : A -> B & { g : B -> A |
    forall b,(compose _ _ _ f g) b = (identity B) b /\
    forall a,(compose _ _ _ g f) a = (identity A) a } }.

Definition is_denumerable A := same_cardinality A nat.

Theorem Q_is_denumerable: is_denumerable Q.