// state-table.h

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Copyright 2005-2010 Google, Inc.
// Author: riley@google.com (Michael Riley)
//
// \file
// Classes for representing the mapping between state tuples and state Ids.

#ifndef FST_LIB_STATE_TABLE_H__
#define FST_LIB_STATE_TABLE_H__

#include <deque>
#include <vector>
using std::vector;

#include <fst/bi-table.h>
#include <fst/expanded-fst.h>


namespace fst {

// STATE TABLES - these determine the bijective mapping between state
// tuples (e.g. in composition triples of two FST states and a
// composition filter state) and their corresponding state IDs.
// They are classes, templated on state tuples, of the form:
//
// template <class T>
// class StateTable {
//  public:
//   typedef typename T StateTuple;
//
//   // Required constructors.
//   StateTable();
//
//   // Lookup state ID by tuple. If it doesn't exist, then add it.
//   StateId FindState(const StateTuple &);
//   // Lookup state tuple by state ID.
//   const StateTuple<StateId> &Tuple(StateId) const;
//   // # of stored tuples.
//   StateId Size() const;
// };
//
// A state tuple has the form:
//
// template <class S>
// struct StateTuple {
//   typedef typename S StateId;
//
//   // Required constructor.
//   StateTuple();
// };


// An implementation using a hash map for the tuple to state ID mapping.
// The state tuple T must have == defined and the default constructor
// must produce a tuple that will never be seen. H is the hash function.
template <class T, class H>
class HashStateTable : public HashBiTable<typename T::StateId, T, H> {
 public:
  typedef T StateTuple;
  typedef typename StateTuple::StateId StateId;
  using HashBiTable<StateId, T, H>::FindId;
  using HashBiTable<StateId, T, H>::FindEntry;
  using HashBiTable<StateId, T, H>::Size;

  HashStateTable() : HashBiTable<StateId, T, H>() {}
  StateId FindState(const StateTuple &tuple) { return FindId(tuple); }
  const StateTuple &Tuple(StateId s) const { return FindEntry(s); }
};


// An implementation using a hash set for the tuple to state ID
// mapping. The state tuple T must have == defined and the default
// constructor must produce a tuple that will never be seen. H is the
// hash function.
template <class T, class H>
class CompactHashStateTable
    : public CompactHashBiTable<typename T::StateId, T, H> {
 public:
  typedef T StateTuple;
  typedef typename StateTuple::StateId StateId;
  using CompactHashBiTable<StateId, T, H>::FindId;
  using CompactHashBiTable<StateId, T, H>::FindEntry;
  using CompactHashBiTable<StateId, T, H>::Size;

  CompactHashStateTable() : CompactHashBiTable<StateId, T, H>() {}

  // Reserves space for table_size elements.
  explicit CompactHashStateTable(size_t table_size)
      : CompactHashBiTable<StateId, T, H>(table_size) {}

  StateId FindState(const StateTuple &tuple) { return FindId(tuple); }
  const StateTuple &Tuple(StateId s) const { return FindEntry(s); }
};

// An implementation using a vector for the tuple to state mapping.
// It is passed a function object FP that should fingerprint tuples
// uniquely to an integer that can used as a vector index. Normally,
// VectorStateTable constructs the FP object.  The user can instead
// pass in this object; in that case, VectorStateTable takes its
// ownership.
template <class T, class FP>
class VectorStateTable
    : public VectorBiTable<typename T::StateId, T, FP> {
 public:
  typedef T StateTuple;
  typedef typename StateTuple::StateId StateId;
  using VectorBiTable<StateId, T, FP>::FindId;
  using VectorBiTable<StateId, T, FP>::FindEntry;
  using VectorBiTable<StateId, T, FP>::Size;
  using VectorBiTable<StateId, T, FP>::Fingerprint;

  explicit VectorStateTable(FP *fp = 0) : VectorBiTable<StateId, T, FP>(fp) {}
  StateId FindState(const StateTuple &tuple) { return FindId(tuple); }
  const StateTuple &Tuple(StateId s) const { return FindEntry(s); }
};


// An implementation using a vector and a compact hash table. The
// selecting functor S returns true for tuples to be hashed in the
// vector.  The fingerprinting functor FP returns a unique fingerprint
// for each tuple to be hashed in the vector (these need to be
// suitable for indexing in a vector).  The hash functor H is used when
// hashing tuple into the compact hash table.
template <class T, class S, class FP, class H>
class VectorHashStateTable
    : public VectorHashBiTable<typename T::StateId, T, S, FP, H> {
 public:
  typedef T StateTuple;
  typedef typename StateTuple::StateId StateId;
  using VectorHashBiTable<StateId, T, S, FP, H>::FindId;
  using VectorHashBiTable<StateId, T, S, FP, H>::FindEntry;
  using VectorHashBiTable<StateId, T, S, FP, H>::Size;
  using VectorHashBiTable<StateId, T, S, FP, H>::Selector;
  using VectorHashBiTable<StateId, T, S, FP, H>::Fingerprint;
  using VectorHashBiTable<StateId, T, S, FP, H>::Hash;

  VectorHashStateTable(S *s, FP *fp, H *h,
                       size_t vector_size = 0,
                       size_t tuple_size = 0)
      : VectorHashBiTable<StateId, T, S, FP, H>(
          s, fp, h, vector_size, tuple_size) {}

  StateId FindState(const StateTuple &tuple) { return FindId(tuple); }
  const StateTuple &Tuple(StateId s) const { return FindEntry(s); }
};


// An implementation using a hash map for the tuple to state ID
// mapping. This version permits erasing of states.  The state tuple T
// must have == defined and its default constructor must produce a
// tuple that will never be seen. F is the hash function.
template <class T, class F>
class ErasableStateTable : public ErasableBiTable<typename T::StateId, T, F> {
 public:
  typedef T StateTuple;
  typedef typename StateTuple::StateId StateId;
  using ErasableBiTable<StateId, T, F>::FindId;
  using ErasableBiTable<StateId, T, F>::FindEntry;
  using ErasableBiTable<StateId, T, F>::Size;
  using ErasableBiTable<StateId, T, F>::Erase;

  ErasableStateTable() : ErasableBiTable<StateId, T, F>() {}
  StateId FindState(const StateTuple &tuple) { return FindId(tuple); }
  const StateTuple &Tuple(StateId s) const { return FindEntry(s); }
};

//
// COMPOSITION STATE TUPLES AND TABLES
//
// The composition state table has the form:
//
// template <class A, class F>
// class ComposeStateTable {
//  public:
//   typedef A Arc;
//   typedef F FilterState;
//   typedef typename A::StateId StateId;
//   typedef ComposeStateTuple<StateId> StateTuple;
//
//   // Required constructors. Copy constructor does not copy state.
//   ComposeStateTable(const Fst<Arc> &fst1, const Fst<Arc> &fst2);
//   ComposeStateTable(const ComposeStateTable<A, F> &table);
//   // Lookup state ID by tuple. If it doesn't exist, then add it.
//   StateId FindState(const StateTuple &);
//   // Lookup state tuple by state ID.
//   const StateTuple<StateId> &Tuple(StateId) const;
//   // # of stored tuples.
//   StateId Size() const;
// };

// Represents the composition state.
template <typename S, typename F>
struct ComposeStateTuple {
  typedef S StateId;
  typedef F FilterState;

  ComposeStateTuple()
      : state_id1(kNoStateId), state_id2(kNoStateId),
        filter_state(FilterState::NoState()) {}

  ComposeStateTuple(StateId s1, StateId s2, const FilterState &f)
      : state_id1(s1), state_id2(s2), filter_state(f) {}

  StateId state_id1;         // State Id on fst1
  StateId state_id2;         // State Id on fst2
  FilterState filter_state;  // State of composition filter
};

// Equality of composition state tuples.
template <typename S, typename F>
inline bool operator==(const ComposeStateTuple<S, F>& x,
                       const ComposeStateTuple<S, F>& y) {
  if (&x == &y)
    return true;
  return x.state_id1 == y.state_id1 &&
      x.state_id2 == y.state_id2 &&
      x.filter_state == y.filter_state;
}


// Hashing of composition state tuples.
template <typename S, typename F>
class ComposeHash {
 public:
  size_t operator()(const ComposeStateTuple<S, F>& t) const {
    return t.state_id1 + t.state_id2 * kPrime0 +
        t.filter_state.Hash() * kPrime1;
  }
 private:
  static const size_t kPrime0;
  static const size_t kPrime1;
};

template <typename S, typename F>
const size_t ComposeHash<S, F>::kPrime0 = 7853;

template <typename S, typename F>
const size_t ComposeHash<S, F>::kPrime1 = 7867;


// A HashStateTable over composition tuples.
template <typename A,
          typename F,
          typename H =
          CompactHashStateTable<ComposeStateTuple<typename A::StateId, F>,
                                ComposeHash<typename A::StateId, F> > >
class GenericComposeStateTable : public H {
 public:
  typedef A Arc;
  typedef typename A::StateId StateId;
  typedef F FilterState;
  typedef ComposeStateTuple<StateId, F> StateTuple;

  GenericComposeStateTable(const Fst<A> &fst1, const Fst<A> &fst2) {}

  GenericComposeStateTable(const GenericComposeStateTable<A, F> &table) {}

 private:
  void operator=(const GenericComposeStateTable<A, F> &table);  // disallow
};


//  Fingerprint for general composition tuples.
template  <typename S, typename F>
class ComposeFingerprint {
 public:
  typedef S StateId;
  typedef F FilterState;
  typedef ComposeStateTuple<S, F> StateTuple;

  // Required, but here useless, constructor.
  ComposeFingerprint() {
    LOG(FATAL) << "TupleFingerprint: # of FST state must be provided.";
  }

  // Constructor is provided the sizes of the input FSTs
  ComposeFingerprint(StateId nstates1, StateId nstates2)
      : mult1_(nstates1), mult2_(nstates1 * nstates2) { }

  size_t operator()(const StateTuple &tuple) {
    return tuple.state_id1 + tuple.state_id2 * mult1_ +
        tuple.filter_state.Hash() * mult2_;
  }

 private:
  ssize_t mult1_;
  ssize_t mult2_;
};


// Useful when the first composition state determines the tuple.
template  <typename S, typename F>
class ComposeState1Fingerprint {
 public:
  typedef S StateId;
  typedef F FilterState;
  typedef ComposeStateTuple<S, F> StateTuple;

  size_t operator()(const StateTuple &tuple) { return tuple.state_id1; }
};


// Useful when the second composition state determines the tuple.
template  <typename S, typename F>
class ComposeState2Fingerprint {
 public:
  typedef S StateId;
  typedef F FilterState;
  typedef ComposeStateTuple<S, F> StateTuple;

  size_t operator()(const StateTuple &tuple) { return tuple.state_id2; }
};


// A VectorStateTable over composition tuples.  This can be used when
// the product of number of states in FST1 and FST2 (and the
// composition filter state hash) is manageable. If the FSTs are not
// expanded Fsts, they will first have their states counted.
template  <typename A, typename F>
class ProductComposeStateTable : public
VectorStateTable<ComposeStateTuple<typename A::StateId, F>,
                 ComposeFingerprint<typename A::StateId, F> > {
 public:
  typedef A Arc;
  typedef typename A::StateId StateId;
  typedef F FilterState;
  typedef ComposeStateTuple<StateId, F> StateTuple;

  ProductComposeStateTable(const Fst<A> &fst1, const Fst<A> &fst2)
      : VectorStateTable<ComposeStateTuple<StateId, F>,
                         ComposeFingerprint<StateId, F> >
  (new ComposeFingerprint<StateId, F>(CountStates(fst1),
                                      CountStates(fst2))) { }

  ProductComposeStateTable(const ProductComposeStateTable<A, F> &table)
      : VectorStateTable<ComposeStateTuple<StateId, F>,
                         ComposeFingerprint<StateId, F> >
        (new ComposeFingerprint<StateId, F>(table.Fingerprint())) {}

 private:
  void operator=(const ProductComposeStateTable<A, F> &table);  // disallow
};

// A VectorStateTable over composition tuples.  This can be used when
// FST1 is a string (satisfies kStringProperties) and FST2 is
// epsilon-free and deterministic. It should be used with a
// composition filter that creates at most one filter state per tuple
// under these conditions (e.g. SequenceComposeFilter or
// MatchComposeFilter).
template <typename A, typename F>
class StringDetComposeStateTable : public
VectorStateTable<ComposeStateTuple<typename A::StateId, F>,
                 ComposeState1Fingerprint<typename A::StateId, F> > {
 public:
  typedef A Arc;
  typedef typename A::StateId StateId;
  typedef F FilterState;
  typedef ComposeStateTuple<StateId, F> StateTuple;

  StringDetComposeStateTable(const Fst<A> &fst1, const Fst<A> &fst2) {
    uint64 props1 = kString;
    uint64 props2 = kIDeterministic | kNoIEpsilons;
    if (fst1.Properties(props1, true) != props1 ||
        fst2.Properties(props2, true) != props2)
      LOG(FATAL) << "StringDetComposeStateTable: fst1 not a string or"
                 << " fst2 not input deterministic and epsilon-free";
  }

  StringDetComposeStateTable(const StringDetComposeStateTable<A, F> &table) {}

 private:
  void operator=(const StringDetComposeStateTable<A, F> &table);  // disallow
};


// A VectorStateTable over composition tuples.  This can be used when
// FST2 is a string (satisfies kStringProperties) and FST1 is
// epsilon-free and deterministic. It should be used with a
// composition filter that creates at most one filter state per tuple
// under these conditions (e.g. SequenceComposeFilter or
// MatchComposeFilter).
template <typename A, typename F>
class DetStringComposeStateTable : public
VectorStateTable<ComposeStateTuple<typename A::StateId, F>,
                 ComposeState1Fingerprint<typename A::StateId, F> > {
 public:
  typedef A Arc;
  typedef typename A::StateId StateId;
  typedef F FilterState;
  typedef ComposeStateTuple<StateId, F> StateTuple;

  DetStringComposeStateTable(const Fst<A> &fst1, const Fst<A> &fst2) {
    uint64 props1 = kODeterministic | kNoOEpsilons;
    uint64 props2 = kString;
    if (fst1.Properties(props1, true) != props1 ||
        fst2.Properties(props2, true) != props2)
      LOG(FATAL) << "StringDetComposeStateTable: fst2 not a string or"
                 << " fst1 not output deterministic and epsilon-free";
  }

  DetStringComposeStateTable(const DetStringComposeStateTable<A, F> &table) {}

 private:
  void operator=(const DetStringComposeStateTable<A, F> &table);  // disallow
};


// An ErasableStateTable over composition tuples. The Erase(StateId) method
// can be called if the user either is sure that composition will never return
// to that tuple or doesn't care that if it does, it is assigned a new
// state ID.
template <typename A, typename F>
class ErasableComposeStateTable : public
ErasableStateTable<ComposeStateTuple<typename A::StateId, F>,
                   ComposeHash<typename A::StateId, F> > {
 public:
  typedef A Arc;
  typedef typename A::StateId StateId;
  typedef F FilterState;
  typedef ComposeStateTuple<StateId, F> StateTuple;

  ErasableComposeStateTable(const Fst<A> &fst1, const Fst<A> &fst2) {}

  ErasableComposeStateTable(const ErasableComposeStateTable<A, F> &table) {}

 private:
  void operator=(const ErasableComposeStateTable<A, F> &table);  // disallow
};

}  // namespace fst

#endif  // FST_LIB_STATE_TABLE_H__
