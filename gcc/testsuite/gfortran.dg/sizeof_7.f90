! { dg-do run }
!
! Check the element length value set in a scalar descriptor associated with a
! polymorphic assumed-rank dummy.  The value set in the descriptor passed to s1
! used to be that of the class descriptor, instead of that of the type t,
! causing failure in s2.

program prog
  implicit none
  type :: t
    integer :: c1
  end type
  type(t) :: x
  class(t), allocatable :: y
  x = t(5)
  y = x
  call s1(y, sizeof(x))
contains
  subroutine s1(a, e)
    class(t), intent(in) :: a(..)
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
