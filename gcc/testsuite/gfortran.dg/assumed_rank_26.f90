! { dg-do compile }
! { dg-additional-options "-fcoarray=single" }
!
! The following program used to cause an ICE because the initialization of the
! array descriptor for the call to `as' was generating a reference to the first
! component of `a', which has none in its declared type.

program prog
  implicit none
  type :: t0
  end type
  type, extends(t0) :: t1
    integer :: c1
  end type
  type(t1) :: x[*]
  x = t1(17)
  call s(x)
contains
  subroutine s(a)
    class(t0), intent(in) :: a[*]
    call as(a)
  end subroutine
  subroutine as(a)
    class(t0), intent(in) :: a(..)
  end subroutine
end program
