! { dg-do compile }
!
! Check that the scalar descriptor initialization used to implement ASSOCIATED
! doesn't trip on the reference-to-array type of its second argument.

program prog
  implicit none
  integer, parameter :: n = 10
  character(n), target :: a
  a = "abcdefghij"
  call s(a, a)
contains
  subroutine s(x, y)
    character(*), pointer, intent(in) :: x(..)
    character(*), target, intent(in) :: y
    if (.not. associated(x, y)) error stop 1
  end subroutine
end program
