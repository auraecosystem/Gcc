! { dg-do run }
!
! Check that scalar values are correctly passed to polymorphic assumed-rank
! dummy arguments.

program prog
   implicit none
   integer, parameter :: n = 5
   type :: t
      integer :: c
   end type
   class(t), allocatable :: a(:)
   integer :: i
   allocate(a(n))
   do i=1,n
      a(i)%c = i*i
   end do
   call sub(a(2),  4, 1)
   call sub(a(3),  9, 2)
   call sub(a(5), 25, 3)
contains
   subroutine sub(x, y, e)
      class(t), intent(in) :: x(..)
      integer, intent(in) :: y, e
      select rank(x)
      rank (0)
         call check(x%c, y, e*10+1)
      rank default
         call check(rank(x), 0, e*10+2)
      end select
   end subroutine
   subroutine check(x, y, e)
      integer, intent(in) :: x, y, e
      if (x /= y) then
         print *, e, "FAIL", x, y
         error stop e
      end if
   end subroutine
end program prog
