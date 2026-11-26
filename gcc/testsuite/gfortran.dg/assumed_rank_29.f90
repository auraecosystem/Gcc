! { dg-do run }
! { dg-additional-options "-fcoarray=single" }
!
! The following program used to cause an ICE because the copy back of the
! array descriptor after the call to `as' was generating a reference to the
! first component of `a', which has none in its declared type.

program prog
  implicit none
  type :: t0
    integer :: c0
  end type
  class(t0), allocatable :: x[:]
  allocate(x[*])
  x%c0 = 17
  call s1(x)
  if (x%c0 /= 9) then
    print *, x%c0
    error stop 21
  end if
contains
  subroutine s1(a)
    class(t0), allocatable :: a(..)
    select rank (a)
    rank (0)
      a%c0 = 9
    rank default
      error stop 1
    end select
  end subroutine
end program
