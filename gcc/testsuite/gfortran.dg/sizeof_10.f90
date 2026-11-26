! { dg-do run }
!
! PR 122521

program prog
  implicit none
  type :: t
    integer :: c(3)
  end type
  class(*), allocatable :: y
  integer :: nfail = 0
  character(len=7) :: c1 = 'abcdefg'
  character(kind=4, len=5) :: c4 = 4_'vwxyz'

  y = c1
  call s1(y, 7_8, 1)
  y = c4
  call s1(y, 20_8, 2)
  y = 42_8
  call s1(y, 8_8, 3)
  y = t([1, 2, 3])
  call s1(y, 12_8, 4)
  if (nfail /= 0) error stop nfail
contains
  subroutine s1(a, e, n)
    class(*), intent(in) :: a(..)
    integer(kind=8), intent(in) :: e
    integer, intent(in) :: n
    call s2(a, e, n)
  end subroutine
  subroutine s2(a, e, n)
    type(*), intent(in) :: a(..)
    integer(kind=8), intent(in) :: e
    integer, intent(in) :: n
    if (sizeof(a) /= e) then
      print *, n, 'sizeof', sizeof(a), 'expected', e
      nfail = nfail + 1
    end if
  end subroutine
end program
