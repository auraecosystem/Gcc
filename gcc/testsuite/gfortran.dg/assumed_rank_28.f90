! { dg-do compile }
! { dg-additional-options "-fcoarray=single" }
!
! The following program used to cause an ICE because the copy back of the
! array descriptor after the call to `as' was generating a reference to the
! first component of `a', which has none in its declared type.

program prog
  implicit none
  type :: t0
  end type
  type, extends(t0) :: t1
    integer :: c1
  end type
  class(t0), allocatable :: x[:]
  allocate(t1 :: x[*])
  select type(x)
  type is (t1)
    x%c1 = 17
  class default
    error stop 1
  end select
  call as(x)
contains
  subroutine as(a)
    class(t0), allocatable :: a(..)
  end subroutine
end program
