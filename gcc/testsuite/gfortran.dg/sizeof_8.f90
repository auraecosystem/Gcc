! { dg-do run }
!
! Check the element length value set in a scalar descriptor associated with a
! polymorphic assumed-rank dummy.  The value set in the descriptor passed to s1
! used to be that of the declared type t1, instead of that of the effective
! type t2, causing failure in s2.

program prog
  implicit none
  type :: t1
    integer :: c1
  end type
  type, extends(t1) :: t2
    integer :: c2
  end type
  type(t2) :: x
  class(t1), allocatable :: y
  x = t2(5, 13)
  y = x
  call s1(y, sizeof(x))
contains
  subroutine s1(a, e)
    class(t1), intent(in) :: a(..)
    integer(kind=8), intent(in) :: e
    call s2(a, e)
  end subroutine
  subroutine s2(a, e)
    type(*), intent(in) :: a(..)
    integer(kind=8), intent(in) :: e
    !print *, sizeof(a), e
    if (sizeof(a) /= e) error stop 1
  end subroutine
end program
