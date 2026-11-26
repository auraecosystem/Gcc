! { dg-do run }
!
! Check the element length value set in a scalar descriptor associated with a
! polymorphic assumed-rank dummy.  The value set in the descriptor passed to s1
! used to be that of the character kind, without the character length scaling,
! causing failure in s2.

program prog
  implicit none
  integer, parameter :: n = 10
  character(len=n), target :: x
  class(*), pointer :: y
  x = "abcdefghij"
  y => x
  call s1(y, n)
contains
  subroutine s1(a, e)
    class(*), intent(in) :: a(..)
    integer, intent(in) :: e
    call s2(a, e)
  end subroutine
  subroutine s2(a, e)
    type(*), intent(in) :: a(..)
    integer, intent(in) :: e
    !print *, sizeof(a), e
    if (sizeof(a) /= int(e)) error stop 1
  end subroutine
end program
