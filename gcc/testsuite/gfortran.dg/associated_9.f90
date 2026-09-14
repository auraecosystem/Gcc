! { dg-do run }
!
! PR fortran/127233
! Check that the intrinsic ASSOCIATED takes the span and the storage size
! into account.

program prog
  implicit none
  character(len=:), allocatable, target :: c(:)
  c = ["abc", "def", "ghi", "jkl", "mno"]
  call sc(c, c, .true., 11)
  call sc(c(::2), c(::2), .true., 12)
  call sc(c(:)(1:2), c(:)(1:2), .true., 13)
  call sc(c(::2)(1:2), c(::2)(1:2), .true., 14)
  call sc(c(1:5:2), c(1:3), .false., 21)
  call sc(c(1:3), c(1:5:2), .false., 22)
  call sc(c(:)(1:2), c(:), .false., 23)
  call sc(c(:), c(:)(1:2), .false., 24)
  call sc(c(1:5:2)(1:2), c(1:3), .false., 25)
  call sc(c(1:3), c(1:5:2)(1:2), .false., 26)
  call sc(c(1:5:2), c(1:3)(1:2), .false., 27)
  call sc(c(1:3)(1:2), c(1:5:2), .false., 28)
contains
  subroutine sc(p, t, r, e)
    character(len=:), pointer, intent(in) :: p(:)
    character(len=*), target,  intent(in) :: t(:)
    logical, intent(in) :: r
    integer, intent(in) :: e
    call check(associated(p, t), r, e)
  end subroutine
  subroutine check(r, v, e)
    logical, intent(in) :: r, v
    integer, intent(in) :: e
    !print *, r, v, (r.eqv.v ? "PASS" : "FAIL"), e
    if (r .neqv. v) error stop e
  end subroutine
end program
